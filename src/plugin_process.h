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
#ifndef __PluginProcess__H_INCLUDED__
#define __PluginProcess__H_INCLUDED__

#include "global.h"
#include "audiobuffer.h"
#include "decimator.h"
#include "filter.h"
#include "limiter.h"
#include "pitchshifter.h"
#include "reverb.h"
#include <vector>

using namespace Steinberg;

namespace Igorski {
class PluginProcess {

    // max delay time in milliseconds
    // this is used when host sync is disabled
    // otherwise max delay time equals a full measure

    const float MAX_DELAY_TIME_MS = 5000.f;

    public:
        PluginProcess( int amountOfChannels );
        ~PluginProcess();

        // apply effect to incoming sampleBuffer contents

        template <typename SampleType>
        void process( SampleType** inBuffer, SampleType** outBuffer, int numInChannels, int numOutChannels,
            int bufferSize, uint32 sampleFramesSize
        );

        // set delay time (in milliseconds)

        void setDelayTime( float value );
        void setDelayFeedback( float value );
        void setDelayMix( float value );

        void setPitchShift( float value );
        void setHarmony( float value, bool syncToBeat );
        void setHarmonyLFOSpeed( float oddSteps, float evenSteps, bool linkLFOs );

        bool isHarmonized() {
            return _harmonize > 0.f;
        }

        void enableReverb( bool enabled ) {
            _reverbEnabled = enabled;
        }

        // synchronize the delay tempo with the host
        // tempo is in BPM, time signature provided as: timeSigNumerator / timeSigDenominator (e.g. 3/4)
        // returns true when tempo has updated, false to indicate no change was made

        bool setTempo( double tempo, int32 timeSigNumerator, int32 timeSigDenominator );

        Decimator* decimator;
        Filter* filter;
        std::vector<PitchShifter*>* _pitchShifters;
        std::vector<Reverb*> _reverbs;
        Limiter* limiter;

        // whether delay time is synced to hosts tempo

        bool syncDelayToHost;

    private:
        AudioBuffer* _delayBuffer;   // contains the delay memory
        AudioBuffer* _preMixBuffer;  // buffer used for the pre-delay effect mixing
        AudioBuffer* _postMixBuffer; // buffer used for the post-delay effect mixing

        int* _delayIndices;

        int _delayTime; // delay time is represented internally in buffer samples
        float _delayMix;
        float _delayFeedback;

        int _amountOfChannels;
        bool _reverbEnabled = false;

        float _pitchShift = 1.f;
        float _harmonize  = 0.f;

        double _tempo;
        int32 _timeSigNumerator;
        int32 _timeSigDenominator;
        float _fullMeasureDuration = 0.f;
        int _fullMeasureSamples    = 0;
        int _halfMeasureSamples    = 0;
        int _beatSamples           = 0;
        int _sixteenthSamples      = 0;

        // ensures the pre- and post mix buffers match the appropriate amount of channels
        // and buffer size. this also clones the contents of given in buffer into the pre-mix buffer
        // the buffers are pooled so this can be called upon each process cycle without allocation overhead

        template <typename SampleType>
        void prepareMixBuffers( SampleType** inBuffer, int numInChannels, int bufferSize );

        // syncs current delay time to musically pleasing intervals synced to host tempo and time signature

        void syncDelayTime();

        bool _linkedLFOs = false;
        float _oddSteps  = 0.f;
        float _evenSteps = 0.f;

        void syncPitchShifterTables( float steps, int resto );
};
}

#include "plugin_process.tcc"

#endif
