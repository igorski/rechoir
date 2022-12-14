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
#ifndef __PARAMIDS_HEADER__
#define __PARAMIDS_HEADER__

enum
{
    // ids for all visual controls
    // these identifiers are mapped to the UI in plugin.uidesc
    // and consumed by controller.cpp to update the model

    kBypassId = 0, // parameter used to bypass the effect processing

// --- AUTO-GENERATED START

    kDelayTimeId = 1,    // Delay time
    kDelayFeedbackId = 2,    // Delay feedback
    kDelayMixId = 3,    // Delay mix
    kPitchShiftId = 4,    // Pitch shift amount
    kHarmonizeId = 5,    // Scale
    kReverbId = 6,    // Freeze
    kDecimatorId = 7,    // Decimation
    kFilterCutoffId = 8,    // Filter cutoff
    kFilterResonanceId = 9,    // Filter resonance
    kSyncShiftId = 10,    // Modulate pitches
    kOddSpeedId = 11,    // Odd channel speed
    kEvenSpeedId = 12,    // Even channel speed
    kLinkLFOsId = 13,    // Sync choir

// --- AUTO-GENERATED END
};

#endif
