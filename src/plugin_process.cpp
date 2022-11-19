/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2022 Igor Zinken - https://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "plugin_process.h"
#include "calc.h"
#include "tablepool.h"
#include "waveforms.h"
#include <math.h>

namespace Igorski {

PluginProcess::PluginProcess( int amountOfChannels ) {

    // cache the waveforms (as sample rate is known to be accurate on PluginProcess construction)

    TablePool::setTable( WaveGenerator::generate( WaveGenerator::WaveForms::SQUARE ), WaveGenerator::WaveForms::SQUARE );

    _delayTime     = 0;
    _delayMix      = .5f;
    _delayFeedback = .1f;

    _delayBuffer   = new AudioBuffer( amountOfChannels, Calc::millisecondsToBuffer( MAX_DELAY_TIME_MS ));
    _delayIndices  = new int[ amountOfChannels ];
    _pitchShifters = new std::vector<PitchShifter*>( amountOfChannels );

    for ( int i = 0; i < amountOfChannels; ++i ) {
        _delayIndices[ i ] = 0;
        _pitchShifters->at( i ) = new PitchShifter( 4, i );

        Reverb* reverb = new Reverb();
        reverb->setWidth( 1.f );
        reverb->setRoomSize( 1.f );

        _reverbs.push_back( reverb );
    }
    _amountOfChannels = amountOfChannels;

    decimator  = new Decimator( 16, 1.f );
    filter     = new Filter();
    limiter    = new Limiter( 10.f, 500.f, .6f );

    // these will be synced to host, see vst.cpp. here we default to 120 BPM in 4/4 time
    _tempo              = 120.0;
    _timeSigNumerator   = 4;
    _timeSigDenominator = 4;

    syncDelayToHost = true;

    // will be lazily created in the process function
    _preMixBuffer  = 0;
    _postMixBuffer = 0;
}

PluginProcess::~PluginProcess() {
    delete[] _delayIndices;
    while ( !_pitchShifters->empty()) {
        delete _pitchShifters->back(), _pitchShifters->pop_back();
    }
    while ( !_reverbs.empty() ) {
        delete _reverbs.back(), _reverbs.pop_back();
    }
    delete _pitchShifters;
    delete _delayBuffer;
    delete _postMixBuffer;
    delete _preMixBuffer;
    delete decimator;
    delete filter;
    delete limiter;

    TablePool::flush();
}

/* setters */

void PluginProcess::setDelayTime( float value )
{
    // maximum delay time (in milliseconds) is specified in MAX_DELAY_TIME_MS when using freeform scaling
    // when the delay is synced to the host, the maximum time is a single measure
    // at the current tempo and time signature

    float delayMaxInMs = ( syncDelayToHost ) ? (( 60.f / _tempo ) * _timeSigDenominator ) * 1000.f : MAX_DELAY_TIME_MS;

    _delayTime = Calc::millisecondsToBuffer( Calc::cap( value ) * delayMaxInMs );

    if ( syncDelayToHost ) {
        syncDelayTime();
    }

    for ( int i = 0; i < _amountOfChannels; ++i ) {
        if ( _delayIndices[ i ] >= _delayTime ) {
            _delayIndices[ i ] = 0;
        }
    }
}

void PluginProcess::setDelayMix( float value )
{
    _delayMix = value;
}

void PluginProcess::setDelayFeedback( float value )
{
    _delayFeedback = value;
}

void PluginProcess::setPitchShift( float value )
{
    float pitch = std::fmin( PitchShifter::OCTAVE_UP, std::fmax( PitchShifter::OCTAVE_DOWN, value ));
    if ( pitch == _pitchShift ) {
        return;
    }

    _pitchShift = pitch;

    if ( _harmonize ) {
        return;
    }
    for ( auto pitchShifter : *_pitchShifters ) {
        pitchShifter->pitchShift = _pitchShift;
    }
}

void PluginProcess::setHarmony( float value, bool syncToBeat )
{
    _harmonize = value;

    if ( !isHarmonized() ) {
        setPitchShift( _pitchShift );
        return;
    }

    // determine scale by integral value
    VST::Scale scale = static_cast<VST::Scale>(( int ) round( 5.f * value ));

    for ( size_t i = 0; i < _pitchShifters->size(); ++i ) {
        _pitchShifters->at( i )->setScale( scale, syncToBeat );
    }
}

void PluginProcess::setHarmonyStepSpeed( float oddSteps, float evenSteps, bool linkGates )
{
    if ( _oddSteps == oddSteps && _evenSteps == evenSteps && _linkedGates == linkGates ) {
       return;
    }
    bool wasLinked = _linkedGates;

    _linkedGates   = linkGates;
    _oddSteps      = oddSteps;
    _evenSteps     = evenSteps;

    // in case the gate speeds are newly synchronized, align the oscillator accumulators
    // TODO should we align the note indices between all channels when gates are linked?

    if ( linkGates && !wasLinked ) {
        for ( size_t i = 0; i < _amountOfChannels; ++i ) {
            bool isEvenChannel = ( i % 2 ) == 1;
            if ( isEvenChannel ) {
                float lastAccumulator = _pitchShifters->at( i - 1 )->getWaveTable()->getAccumulator();
                _pitchShifters->at( i )->getWaveTable()->setAccumulator( lastAccumulator );
            }
        }
    }
    syncPitchShifterTables( oddSteps, 0 );
    syncPitchShifterTables( linkGates ? oddSteps : evenSteps, 1 );
}

bool PluginProcess::setTempo( double tempo, int32 timeSigNumerator, int32 timeSigDenominator )
{
    if ( _tempo == tempo && _timeSigNumerator == timeSigNumerator && _timeSigDenominator == timeSigDenominator ) {
        return false; // no change
    }

    if ( syncDelayToHost ) {

        // if delay is synced to host tempo, keep delay time
        // relative to new tempo

        float currentFullMeasureDuration = ( 60.f / _tempo ) * _timeSigDenominator;
        float currentDelaySubdivision    = currentFullMeasureDuration / _delayTime;

        // calculate new delay time (note we're using passed arguments as values)

        float newFullMeasureDuration = ( 60.f / tempo ) * timeSigDenominator;
        _delayTime = newFullMeasureDuration / currentDelaySubdivision;
    }

    _timeSigNumerator   = timeSigNumerator;
    _timeSigDenominator = timeSigDenominator;
    _tempo              = tempo;

    _fullMeasureDuration = ( 60.f / _tempo ) * _timeSigDenominator; // seconds per measure
    _fullMeasureSamples  = Calc::secondsToBuffer( _fullMeasureDuration ); // samples per measure
    _beatSamples         = ceil( _fullMeasureSamples / _timeSigDenominator ); // samples per beat
    _halfMeasureSamples  = ceil( _fullMeasureSamples / 2 ); // samples per half measure
    _sixteenthSamples    = ceil( _fullMeasureSamples / 16 ); // samples per 16th note

    return true;
}

/* protected methods */

void PluginProcess::syncDelayTime()
{
    // duration of a full measure in samples

    int fullMeasureSamples = Calc::secondsToBuffer(( 60.f / _tempo ) * _timeSigDenominator );

    // we allow syncing to up to 32nd note resolution

    int subdivision = 32;

    _delayTime = Calc::roundTo( _delayTime, fullMeasureSamples / subdivision );
}

void PluginProcess::syncPitchShifterTables( float steps, int resto )
{
    // (1.f / measureDuration value) converts seconds to cycles in Hertz
    float value = 1.f / ( _fullMeasureDuration / Calc::gateSubdivision( steps ));

    for ( size_t i = 0; i < _amountOfChannels; ++i ) {
        if (( i % 2 ) == resto ) {
            _pitchShifters->at( i )->getWaveTable()->setFrequency( value );
        }
    }
}

}
