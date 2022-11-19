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
#include <math.h>

namespace Igorski {

PluginProcess::PluginProcess( int amountOfChannels ) {
    _delayTime     = 0;
    _delayMix      = .5f;
    _delayFeedback = .1f;

    _delayBuffer   = new AudioBuffer( amountOfChannels, Calc::millisecondsToBuffer( MAX_DELAY_TIME_MS ));
    _delayIndices  = new int[ amountOfChannels ];
    _pitchShifters = new std::vector<PitchShifter*>( amountOfChannels );

    for ( int i = 0; i < amountOfChannels; ++i ) {
        _delayIndices[ i ] = 0;
        _pitchShifters->at( i ) = new PitchShifter( 4 );

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

void PluginProcess::setHarmony( float value )
{
    _harmonize = value;

    if ( !isHarmonized() ) {
        setPitchShift( _pitchShift );
        return;
    }

    // determine scale by integral value

    float odd  = 1.f;
    float even = 1.f;
    int scaled = ( int ) round( 5.f * value );

    // TODO: only shifting up ?
    switch ( scaled ) {
        // "neutral"
        default:
        case 0:
            odd  = 7; // 5th
            even = 2; // 2nd
            break;
        // major
        case 1:
            odd  = 11; // major 7th
            even = 4; // major 3rd
            break;
        // mixolydian
        case 2:
            odd  = 10; // minor 7th
            even = 4; // major 3rd
            break;
        // augmented
        case 3:
            odd  = 8; // augmented 5th
            even = 4; // major 3rd
            break;
        // minor
        case 4:
            odd  = 10; // minor 7th
            even = 3; // minor 3rd
            break;
        // diminished
        case 5:
            odd  = 6; // diminished 5th / tritone
            even = 3; // minor 3rd
            break;
    }
    // formula for pitching down is pow( 0.94387f, -semitones )

    float oddPitch  = pow( 1.05946f, odd );
    float evenPitch = pow( 1.05946f, even );

    for ( size_t i = 0; i < _pitchShifters->size(); ++i ) {
        _pitchShifters->at( i )->pitchShift = ( i % 2 == 0 ) ? evenPitch : oddPitch;
    }
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

}
