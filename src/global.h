#ifndef __GLOBAL_HEADER__
#define __GLOBAL_HEADER__

#include "pluginterfaces/base/fplatform.h"
#include "pluginterfaces/base/funknown.h"

using namespace Steinberg;

namespace Igorski {
namespace VST {

    static const int   ID       = 12345678;
    static const char* NAME     = "__PLUGIN_NAME__";
    static const char* VENDOR   = "igorski.nl";

    // generate unique UIDs for these (www.uuidgenerator.net is great for this)

    static const FUID PluginProcessorUID( 0xC6E40BB6, 0x717148FB, 0x92700948, 0x0C47f4E8 );
    static const FUID PluginWithSideChainProcessorUID( 0x717148FB, 0x92700948, 0x0C47f4E8, 0xC6E40BB6 );
    static const FUID PluginControllerUID( 0x92700948, 0x0C47f4E8, 0xC6E40BB6, 0x717148FB );

    extern float SAMPLE_RATE; // set upon initialization, see vst.cpp

    // maximum and minimum filter frequency ranges
    // also see .uidesc to update the controls to match

    static const float FILTER_MIN_FREQ      = 30.f;
    static const float FILTER_MAX_FREQ      = 22050.f;
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
        MINOR,
        DIMINISHED
    };

    // scale notes (in order of Scale enum!)

    static constexpr int SCALE_NOTES[ 6 ][ NOTES_IN_SCALE ] = {
        { 2, 7 },  // 2nd and 5th
        { 4, 11 }, // major 3rd and major 7th
        { 4, 10 }, // major 3rd and minor 7th
        { 4, 8 },  // minor 3rd and augmented 5th
        { 3, 10 }, // minor 3rd and minor 7th
        { 3, 6 }   // minor 3rd and diminished 5th
    };
}
}

#endif
