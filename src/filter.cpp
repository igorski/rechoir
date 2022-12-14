/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
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
#include "filter.h"
#include "global.h"
#include <algorithm>

using namespace Steinberg;

namespace Igorski {

Filter::Filter() {

    _cutoff    = VST::FILTER_MIN_FREQ;
    _resonance = VST::FILTER_MIN_RESONANCE;

    _a1 = 0.f;
    _a2 = 0.f;
    _a3 = 0.f;
    _b1 = 0.f;
    _b2 = 0.f;
    _c  = 0.f;

    // stereo (2) probably enough...
    int numChannels = 8;

    _in1  = new float[ numChannels ];
    _in2  = new float[ numChannels ];
    _out1 = new float[ numChannels ];
    _out2 = new float[ numChannels ];

    for ( int i = 0; i < numChannels; ++i )
    {
        _in1 [ i ] = 0.f;
        _in2 [ i ] = 0.f;
        _out1[ i ] = 0.f;
        _out2[ i ] = 0.f;
    }
    setCutoff( VST::FILTER_MAX_FREQ / 2 );
}

Filter::~Filter() {
    delete[] _in1;
    delete[] _in2;
    delete[] _out1;
    delete[] _out2;
}

/* public methods */

void Filter::updateProperties( float cutoffPercentage, float resonancePercentage )
{
    float co  = VST::FILTER_MIN_FREQ + ( cutoffPercentage * ( VST::FILTER_MAX_FREQ - VST::FILTER_MIN_FREQ ));
    float res = VST::FILTER_MIN_RESONANCE + ( resonancePercentage * ( VST::FILTER_MAX_RESONANCE - VST::FILTER_MIN_RESONANCE ));

    if ( _cutoff != co || _resonance != res ) {
        setCutoff( co );
        setResonance( res );
    }
}

void Filter::process( float* sampleBuffer, int bufferSize, int c )
{
    for ( int32 i = 0; i < bufferSize; ++i )
    {
        float input  = sampleBuffer[ i ];
        float output = _a1 * input + _a2 * _in1[ c ] + _a3 * _in2[ c ] - _b1 * _out1[ c ] - _b2 * _out2[ c ];

        _in2 [ c ] = _in1[ c ];
        _in1 [ c ] = input;
        _out2[ c ] = _out1[ c ];
        _out1[ c ] = output;

        // commit the effect
        sampleBuffer[ i ] = output;
    }
}

void Filter::setCutoff( float frequency )
{
    _cutoff = std::max( VST::FILTER_MIN_FREQ, std::min( frequency, VST::FILTER_MAX_FREQ ));
    calculateParameters();
}

float Filter::getCutoff()
{
    return _cutoff;
}

void Filter::setResonance( float resonance )
{
    _resonance = std::max( VST::FILTER_MIN_RESONANCE, std::min( resonance, VST::FILTER_MAX_RESONANCE ));
    calculateParameters();
}

float Filter::getResonance()
{
    return _resonance;
}

void Filter::calculateParameters()
{
    _c  = 1.f / tan( VST::PI * _cutoff / VST::SAMPLE_RATE );
    _a1 = 1.f / ( 1.f + _resonance * _c + _c * _c );
    _a2 = 2.f * _a1;
    _a3 = _a1;
    _b1 = 2.f * ( 1.f - _c * _c ) * _a1;
    _b2 = ( 1.f - _resonance * _c + _c * _c ) * _a1;
}

}
