#ifndef __GLOBAL_HEADER__
#define __GLOBAL_HEADER__

#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/base/funknown.h"

using namespace Steinberg;

namespace Igorski {
namespace VST {

    static const int   ID       = 97151822;
    static const char* NAME     = "Rechoir";
    static const char* VENDOR   = "igorski.nl";

    static const FUID PluginProcessorUID( 0xB5D40EF0, 0xD98545BB, 0xBE1C9262, 0x4F78C208 );
    static const FUID PluginWithSideChainProcessorUID( 0xD98545BB, 0xBE1C9262, 0x4F78C208, 0xB5D40EF0 );
    static const FUID PluginControllerUID( 0xBE1C9262, 0x4F78C208, 0xB5D40EF0, 0xD98545BB );

    extern float SAMPLE_RATE; // set upon initialization, see vst.cpp

    // maximum and minimum filter frequency ranges

    static const float FILTER_MIN_FREQ      = 30.f;
    static const float FILTER_MAX_FREQ      = 20000.f;
    static const float FILTER_MIN_RESONANCE = 0.1f;
    static const float FILTER_MAX_RESONANCE = 0.7071067811865476f; //sqrt( 2.f ) / 2.f;

    static const float PI     = 3.141592653589793f;
    static const float TWO_PI = PI * 2.f;

    // maximum and minimum rate of oscillation in Hz
    // also see plugin.uidesc to update the controls to match

    static const float MAX_LFO_RATE() { return 10.f; }
    static const float MIN_LFO_RATE() { return .1f; }

    // these values are tuned to 44.1 kHz sample rate and will be
    // recalculated to match the host sample recalculated

    static const int NUM_COMBS     = 8;
    static const int NUM_ALLPASSES = 4;

    static const int COMB_TUNINGS[ NUM_COMBS ] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
    static const int ALLPASS_TUNINGS[ NUM_ALLPASSES ] = { 556, 441, 341, 225 };

    // intervals for melodic scales

    static const int NOTES_IN_SCALE = 2;

    enum Scale {
        NEUTRAL,
        MAJOR,
        MIXOLYDIAN,
        AUGMENTED,
        DORIAN,
        MINOR,
        DIMINISHED
    };

    // scale notes (in order of Scale enum!)

    static constexpr int SCALE_NOTES[ 7 ][ NOTES_IN_SCALE ] = {
        { 2, 7 },  // 2nd and 5th
        { 4, 11 }, // major 3rd and major 7th
        { 4, 10 }, // major 3rd and minor 7th
        { 4, 8 },  // minor 3rd and augmented 5th
        { 3, 9 },  // minor 3rd and 6th
        { 3, 10 }, // minor 3rd and minor 7th
        { 3, 6 }   // minor 3rd and diminished 5th
    };
}
}

#endif
