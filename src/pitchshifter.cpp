/**
 * The MIT License (MIT)
 *
 * 2013-2022 Igor Zinken - https://www.igorski.nl
 * Based on:
 *
 * NAME: smbPitchShift.cpp
 * VERSION: 1.2
 * HOME URL: http://blogs.zynaptiq.com/bernsee
 *
 * by S.M. Bernsee (1999-2015)
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
#include "pitchshifter.h"
#include "global.h"
#include "tablepool.h"
#include "wavegenerator.h"

namespace Igorski {

/* constructors / destructors */

PitchShifter::PitchShifter( long osampAmount, int instanceNum )
{
    osamp = std::fmax( 4L, osampAmount ); // at least 4 for moderate scaling, 32 for max. quality

    _noteIndex = instanceNum % VST::NOTES_IN_SCALE;

    stepSize      = FFT_FRAME_SIZE / osamp;
    freqPerBin    = ( float ) VST::SAMPLE_RATE / ( float ) FFT_FRAME_SIZE;
    expct         = VST::TWO_PI * ( float ) stepSize /( float ) FFT_FRAME_SIZE;
    inFifoLatency = FFT_FRAME_SIZE - stepSize;

    fftFrameSizeLog = std::log( FFT_FRAME_SIZE );

    /* initialize our static arrays */

    memset( gInFIFO,      0, sizeof( float ) * MAX_FRAME_LENGTH );
    memset( gOutFIFO,     0, sizeof( float ) * MAX_FRAME_LENGTH );
    memset( gFFTworksp,   0, sizeof( float ) * 2 * MAX_FRAME_LENGTH );
    memset( gLastPhase,   0, sizeof( float ) * ( MAX_FRAME_LENGTH / 2 + 1 ));
    memset( gSumPhase,    0, sizeof( float ) * ( MAX_FRAME_LENGTH / 2 + 1 ));
    memset( gOutputAccum, 0, sizeof( float ) * 2 * MAX_FRAME_LENGTH );
    memset( gAnaFreq,     0, sizeof( float ) * MAX_FRAME_LENGTH );
    memset( gAnaMagn,     0, sizeof( float ) * MAX_FRAME_LENGTH );

    // setup square wave based beat LFO to adjust harmony

    _waveTable = TablePool::getTable( WaveGenerator::WaveForms::SQUARE )->clone();
}

PitchShifter::~PitchShifter()
{
    delete _waveTable;
}

/* public methods */

void PitchShifter::setPitchShift( float pitch, bool setDirect )
{
    _baseShift = pitch;

    if ( setDirect ) {
        _pitchShift = pitch;
    }
}

void PitchShifter::setScale( VST::Scale scale )
{
    if ( scale != _scale ) {
        _scale = scale;
        alignPitchToScaleNote();
    }
}

void PitchShifter::syncShiftToLFO( bool syncActive )
{
    _syncShiftToLFO = syncActive;
}

WaveTable* PitchShifter::getWaveTable()
{
    return _waveTable;
}

void PitchShifter::process( float* channelBuffer, int bufferSize )
{
    int i, n, t;
    long k;

    if ( gRover == 0 ) {
        gRover = inFifoLatency;
    }
    float fftFrameSizeFloat = ( float ) FFT_FRAME_SIZE;

    invFftFrameSizePI2 = VST::TWO_PI / FFT_FRAME_SIZE;
    invFftFrameSize2   = 2 / ( FFT_FRAME_SIZE_HALF * osamp );
    osampPI2           = osamp / VST::TWO_PI;

    /* main processing loop */

    for ( i = 0; i < bufferSize; ++i )
    {
        // run the beat sync

        float lfoLevel = _waveTable->peek();

        if (( _isOpen && lfoLevel < 0 ) || ( !_isOpen && lfoLevel > 0 )) {
            handleLFOBeatTrigger();
        }

        // pitch shift at neutral value ? omit processing and save CPU cycles

        if ( _pitchShift == UNCHANGED ) {
            continue;
        }

        // read incoming audio data and into buffer for analysis

        gInFIFO[ gRover ]  = channelBuffer[ i ];
        channelBuffer[ i ] = gOutFIFO[ gRover - inFifoLatency ];
        gRover++;

        if ( gRover < FFT_FRAME_SIZE ) {
            continue; // need more input before we can go on
        }

        gRover = inFifoLatency;

        // windowing and re,im interleaving

/*              TODO: optimize window calculation like below
        for ( k = 0, n = 0, t = 0; k < FFT_FRAME_SIZE; ++k, ++n, ++n, t += invFftFrameSizePI2 )                {
            window = -0.5 * cos(( float ) t ) + 0.5;
            gFFTworksp[ n ]     = gInFIFO[ k ] * window;
            gFFTworksp[ n + 1 ] = 0.0;
        }
*/
        for ( k = 0, n = 0; k < FFT_FRAME_SIZE; ++k, n += 2 )
        {
            window              = -0.5f * Calc::fastCos( VST::TWO_PI * ( float ) k / fftFrameSizeFloat ) + 0.5f;
            gFFTworksp[ n ]     = gInFIFO[ k ] * window;
            gFFTworksp[ n + 1 ] = 0.f;
        }

        // ***************** ANALYSIS *******************

        // transform
        smbFft( gFFTworksp, FFT_FRAME_SIZE, fftFrameSizeLog, -1 );

        // analysis

        for ( k = 0; k <= FFT_FRAME_SIZE_HALF; ++k )
        {
            // de-interlace FFT buffer
            real = gFFTworksp[ k << 1 ];         // [ 2 * k ]
            imag = gFFTworksp[ ( k << 1 ) + 1 ]; // [ ( 2 * k ) + 1 ]

            // compute magnitude and phase
            magn  = 2.f * sqrt( real * real + imag * imag );
            phase = Calc::fastAtan2( imag, real );

            // compute phase difference
            tmp             = phase - gLastPhase[ k ];
            gLastPhase[ k ] = phase;

            // subtract expected phase difference
            tmp -= ( float ) k * expct;

            // map delta phase into +/- Pi interval
            qpd = tmp / VST::PI;
            if ( qpd >= 0 ) {
                qpd += qpd & 1;
            } else {
                qpd -= qpd & 1;
            }
            tmp -= VST::PI * ( float ) qpd;

            // get deviation from bin frequency from the +/- Pi interval
            tmp *= osampPI2;

            // compute the k-th partials' true frequency
            tmp = (( float ) k + tmp ) * freqPerBin;

            // store magnitude and true frequency in analysis arrays
            gAnaMagn[ k ] = magn;
            gAnaFreq[ k ] = tmp;
        }

        /* ***************** PROCESSING ******************* */
        /* this does the actual pitch shifting */
        memset( gSynMagn, 0, sizeof( float ) * FFT_FRAME_SIZE );
        memset( gSynFreq, 0, sizeof( float ) * FFT_FRAME_SIZE );

        for ( k = 0; k <= FFT_FRAME_SIZE_HALF; ++k ) {
            index = k * _pitchShift;
            if ( index <= FFT_FRAME_SIZE_HALF ) {
                gSynMagn[ index ] += gAnaMagn[ k ];
                gSynFreq[ index ]  = gAnaFreq[ k ] * _pitchShift;
            }
        }

        /* ***************** SYNTHESIS ******************* */
        /* this is the synthesis step */
        for ( k = 0; k <= FFT_FRAME_SIZE_HALF; ++k ) {
            /* get magnitude and true frequency from synthesis arrays */
            magn = gSynMagn[ k ];
            tmp  = gSynFreq[ k ];

            /* subtract bin mid frequency */
            tmp -= ( float ) k * freqPerBin;

            /* get bin deviation from freq deviation */
            tmp /= freqPerBin;

            /* take osamp into account */
            tmp = VST::TWO_PI * tmp / osamp;

            /* add the overlap phase advance back in */
            tmp += ( float ) k * expct;

            /* accumulate delta phase to get bin phase */
            gSumPhase[ k ] += tmp;
            phase           = gSumPhase[ k ];

            /* get real and imag part and re-interleave */
            gFFTworksp[ k << 1 ] = magn * Calc::fastCos( phase );     // [ 2 * k ]
            gFFTworksp[ ( k << 1 ) + 1 ] = magn * Calc::fastSin( phase ); // [ (2 * k) + 1 ]
        }

        // zero negative frequencies
        for ( k = FFT_FRAME_SIZE + 2; k < MAX_FRAME_LENGTH; ++k ) {
            gFFTworksp[ k ] = 0.;
        }

        // inverse transform
        smbFft( gFFTworksp, FFT_FRAME_SIZE, fftFrameSizeLog, 1 );

        /* do windowing and add to output accumulator */
        /*
        TODO: optimize window calculation like below
        for ( k = 0, n = 0, t = 0; k < FFT_FRAME_SIZE; ++k, ++n, ++n, t += invFftFrameSizePI2 ){
            window             = -0.5 * cos(( float ) t ) + 0.5;
            gOutputAccum[ k ] += window * gFFTworksp[ n ] * invFftFrameSize2;
        }
        */
        for ( k = 0; k < FFT_FRAME_SIZE; ++k )
        {
            window = -0.5f * Calc::fastCos( VST::TWO_PI * ( float ) k / fftFrameSizeFloat ) + 0.5f;
            gOutputAccum[ k ] += 2.f * window * gFFTworksp[ 2 * k ] / ( FFT_FRAME_SIZE_HALF * osamp );
        }

        for ( k = 0; k < stepSize; ++k ) {
            gOutFIFO[ k ] = gOutputAccum[ k ];
        }

        // shift accumulator
        memmove( gOutputAccum, gOutputAccum + stepSize, FFT_FRAME_SIZE * sizeof( float ));

        // move input FIFO
        for ( k = 0; k < inFifoLatency; ++k ) {
            gInFIFO[ k ] = gInFIFO[ k + stepSize ];
        }
    }
}

/* private methods */

void PitchShifter::handleLFOBeatTrigger()
{
    _isOpen = !_isOpen; // update LFO "gate" status

    if ( !_syncShiftToLFO ) {
        return;
    }

    // update current shift

    alignPitchToScaleNote();

    if ( ++_noteIndex >= VST::NOTES_IN_SCALE ) {
        _noteIndex = 0;
    }
}

void PitchShifter::alignPitchToScaleNote()
{
    if ( _scale == VST::Scale::CUSTOM ) {
        // note the second pitch is slightly above UNCHANGED to prevent pops when switching between pitches
        _pitchShift = _noteIndex == 0 ? _baseShift : 1.0001f;
        return;
    }
    int interval = VST::SCALE_NOTES[ _scale ][ _noteIndex ];
    _pitchShift = ( interval >= 0 ) ? pow( 1.05946f, interval ) : pow( 0.94387f, interval );
}

} // E.O. namespace Igorski
