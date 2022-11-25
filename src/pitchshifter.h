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
#ifndef __PITCHSHIFTER_H_INCLUDED__
#define __PITCHSHIFTER_H_INCLUDED__

#include <string.h>
#include <stdio.h>
#include "calc.h"
#include "wavetable.h"

#define MAX_FRAME_LENGTH 4096//8192

namespace Igorski {

class PitchShifter
{
    /**
     * Defines the FFT frame size used for the processing. Value must be a power
     * of 2 and smaller than or equal to MAX_FRAME_LENGTH with typical values
     * being 1024, 2048 and 4096.
     */
    static const long FFT_FRAME_SIZE      = MAX_FRAME_LENGTH / 2;
    static const long FFT_FRAME_SIZE_HALF = FFT_FRAME_SIZE / 2;

    public:

        static constexpr float UNCHANGED   = 1.f;
        static constexpr float OCTAVE_DOWN = 0.5f;
        static constexpr float OCTAVE_UP   = 2.f;

        /**
         * provided osampAmount is the STFT oversampling factor which also determines
         * the overlap between adjacent STFT frames. It should at least be 4 for moderate
         * scaling ratios. A value of 32 is recommended for best quality.
         */
        PitchShifter( long osampAmount, int instanceNum );
        ~PitchShifter();

        // 0.5 is octave down, 1 == normal, 2 is octave up
        // The routine takes a pitchShift factor value which is between 0.5 (one octave down)
        // and 2. (one octave up). A value of exactly 1 does not change the pitch

        void setPitchShift( float pitch, bool setDirect );
        void setScale( VST::Scale scale );
        void syncShiftToLFO( bool syncActive );

        WaveTable* getWaveTable();
        void process( float* channelBuffer, int bufferSize );

    private:
        float gInFIFO     [ MAX_FRAME_LENGTH ];
        float gOutFIFO    [ MAX_FRAME_LENGTH ];
        float gFFTworksp  [ 2 * MAX_FRAME_LENGTH ];
        float gLastPhase  [ MAX_FRAME_LENGTH / 2 + 1 ];
        float gSumPhase   [ MAX_FRAME_LENGTH / 2 + 1 ];
        float gOutputAccum[ 2 * MAX_FRAME_LENGTH ];
        float gAnaFreq    [ MAX_FRAME_LENGTH ];
        float gAnaMagn    [ MAX_FRAME_LENGTH ];
        float gSynFreq    [ MAX_FRAME_LENGTH ];
        float gSynMagn    [ MAX_FRAME_LENGTH ];

        float _pitchShift = UNCHANGED; // current pitch shift value
        float _baseShift  = UNCHANGED; // base pitch shift value to return to when syncing

        long gRover = false;

        float magn, phase, tmp, window, real, imag, freqPerBin, expct, invFftFrameSizePI2, invFftFrameSize2, osampPI2;
        long qpd, index, inFifoLatency, stepSize, osamp;

        // inlining this FFT routine (by S.M. Bernsee, 1996) provides a 21% performance boost

        inline void smbFft( float *fftBuffer, long fftFrameSize, long sign )
        {
            float wr, wi, arg, *p1, *p2, temp;
            float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
            long doubleFftFrameSize = 2 * fftFrameSize, i, end, bitm, j, le, le2, k;

            for ( i = 2, end = doubleFftFrameSize - 2; i < end; i += 2 ) {
                for ( bitm = 2, j = 0; bitm < doubleFftFrameSize; bitm <<= 1 ) {
                    if ( i & bitm ) ++j;
                    j <<= 1;
                }
                if ( i < j ) {
                    p1        = fftBuffer+i;
                    p2        = fftBuffer+j;
                    temp      = *p1;
                    *( p1++ ) = *p2;
                    *( p2++ ) = temp;
                    temp      = *p1;
                    *p1       = *p2;
                    *p2       = temp;
                }
            }

            // 0.69314... being std::log( 2.f )
            for ( k = 0, le = 2, end = ( long )( std::log( fftFrameSize ) / 0.6931471805599453f + .5f ); k < end; ++k )
            {
                le <<= 1;
                le2  = le >> 1;
                ur  = 1.0;
                ui  = 0.0;
                arg = M_PI / ( le2 >> 1 );
                wr  = std::cos( arg );
                wi  = sign * std::sin( arg );

                for ( j = 0; j < le2; j += 2 ) {
                    p1r = fftBuffer + j;
                    p1i = p1r + 1;
                    p2r = p1r + le2;
                    p2i = p2r + 1;

                    for ( i = j; i < doubleFftFrameSize; i += le ) {
                        tr    = *p2r * ur - *p2i * ui;
                        ti    = *p2r * ui + *p2i * ur;
                        *p2r  = *p1r - tr;
                        *p2i  = *p1i - ti;
                        *p1r += tr;
                        *p1i += ti;
                        p1r  += le;
                        p1i  += le;
                        p2r  += le;
                        p2i  += le;
                    }
                    tr = ur * wr - ui * wi;
                    ui = ur * wi + ui * wr;
                    ur = tr;
                }
            }
        }

        WaveTable* _waveTable = nullptr;

        float _isOpen        = false;
        VST::Scale _scale    = VST::Scale::NEUTRAL;
        bool _syncShiftToLFO = false;
        int _noteIndex       = 0;

        void alignPitchToScaleNote();
        void handleLFOBeatTrigger();
};
} // E.O. namespace Igorski

#endif
