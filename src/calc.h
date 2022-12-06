/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2022 Igor Zinken - https://www.igorski.nl
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
#ifndef __CALC_HEADER__
#define __CALC_HEADER__

#include <cmath>
#include <algorithm>
#include "global.h"

// internally we handle all audio as 32-bit floats (hence 0x7f800000)
// this methods is used by comb and allpass
#define undenormalise(sample) ((((*(uint32 *)&(sample))&0x7f800000)==0)&&((sample)!=0.f))

/**
 * convenience utilities to process values
 * common to the VST plugin context
 */
namespace Igorski {
namespace Calc {

    /**
     * convert given value in seconds to the appropriate
     * value in samples (for the current sampling rate)
     */
    inline int secondsToBuffer( float seconds )
    {
        return ( int )( seconds * Igorski::VST::SAMPLE_RATE );
    }

    /**
     * convert given value in milliseconds to the appropriate
     * value in samples (for the current sampling rate)
     */
    inline int millisecondsToBuffer( float milliseconds )
    {
        return secondsToBuffer( milliseconds / 1000.f );
    }

    // convenience method to ensure given value is within the 0.f - +1.f range

    inline float cap( float value )
    {
        return std::min( 1.f, std::max( 0.f, value ));
    }

    // convenience method to ensure a sample is in the valid -1.f - +1.f range

    inline float capSample( float value )
    {
        return std::min( 1.f, std::max( -1.f, value ));
    }

    // convenience method to round given number value to the nearest
    // multiple of valueToRoundTo
    // e.g. roundTo( 236.32, 10 ) == 240 and roundTo( 236.32, 5 ) == 235

    inline float roundTo( float value, float valueToRoundTo )
    {
        float resto = fmod( value, valueToRoundTo );

        if ( resto <= ( valueToRoundTo / 2 ))
            return value - resto;

        return value + valueToRoundTo - resto;
    }

    // inverts a 0 - 1 normalized min-to-max value to have 0 be the max and 1 the min

    inline float inverseNormalize( float value )
    {
        return ( 1.f - value ) / 1.f;
    }

    // convenience method to scale given value and its expected maxValue against
    // an arbitrary range (defined by maxCompareValue in relation to maxValue)

    inline float scale( float value, float maxValue, float maxCompareValue )
    {
        float ratio = maxCompareValue / maxValue;
        return ( float ) ( std::min( maxValue, value ) * ratio );
    }

    // cast a floating point value to a boolean true/false

    inline bool toBool( float value )
    {
        return value >= .5;
    }

    // converts the normalized 0 - 1 value for the gate
    // into musical subdivisions from 4 measures, 2 measures, 1 measure
    // and individual (num / measure) down to a 16th note in duration

    inline float gateSubdivision( float value )
    {
        int scaled = ( int ) round(( 18.f * value ) + 1.f );
        switch ( scaled ) {
            case 1:
                return 0.25f;
            case 2:
                return 0.5f;
            case 3:
                return 1;
            default:
                return -round( 1.f / 16.f - (( float ) scaled - 3.f ));
        }
    }

    // faster alternatives for sine, cosine, atan2 and natural log calculations
    // there will be some margins of error, but depending on use
    // case, these can be quite acceptable though be careful before
    // applying these anywhere in a DSP process

    inline float fastCos( float x )
    {
        x *= 0.15915494309189535f; // 1 / ( 2 * PI );
        x -= 0.25f + static_cast<int>( x + 0.25f );
        x *= 16.f * (( x >= 0 ? x : -x ) - 0.5f );
        // below adds additional precision
        x += 0.225f * x * ( std::abs( x ) - 1.f );

        return x;
    }

    inline float fastSin( float x )
    {
        return fastCos( x - 1.5707963267948966f ); // PI / 2
    }

    inline float fastAtan2( float y, float x )
    {
        // http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
        // Volkan SALMA
        /*
    	float r, angle;
    	float abs_y = fabs( y ) + 1e-10f; // kludge to prevent 0/0 condition

        if ( x < 0.0f ) {
    		r = ( x + abs_y ) / ( abs_y - x );
    		angle = 2.356194490192345f; // 3.0 * M_PI / 4.0
    	} else {
    		r = ( x - abs_y ) / ( x + abs_y );
    		angle = 0.7853981633974483f; // ( M_PI / 4.0 )
    	}
    	angle += ( 0.1963f * r * r - 0.9817f ) * r;
    	if ( y < 0.0f ) {
    		return( -angle ); // negate if in quad III or IV
    	} else {
    		return( angle );
        }*/
        float abs_y = std::fabs( y ) + 1e-10f; // kludge to prevent 0/0 condition
    	float r = ( x - std::copysign( abs_y, x )) / ( abs_y + std::fabs( x ));
    	float angle = M_PI/2.f - std::copysign( 0.7853981633974483f /* M_PI / 4.0 */, x );

    	angle += ( 0.1963f * r * r - 0.9817f ) * r;

    	return std::copysign( angle, y );
    }

    inline float fastLog( float x )
    {
        unsigned int bx = * ( unsigned int* ) ( &x );
        unsigned int ex = bx >> 23;
        signed int t    = ( signed int ) ex - ( signed int ) 127;
        unsigned int s  = ( t < 0 ) ? ( -t ) : t;
        bx = 1065353216 | ( bx & 8388607 );
        x  = * ( float* ) ( &bx );

        return -1.49278f + ( 2.11263f + ( -0.729104f + 0.10969f * x ) * x ) * x + 0.6931471806f * t;
    }
}
}

#endif
