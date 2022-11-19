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
#include "global.h"
#include "vst.h"
#include "paramids.h"
#include "calc.h"

#include "public.sdk/source/vst/vstaudioprocessoralgo.h"

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/vstpresetkeys.h"

#include "base/source/fstreamer.h"

#include <stdio.h>

namespace Igorski {

float VST::SAMPLE_RATE = 44100.f; // updated in setupProcessing()

//------------------------------------------------------------------------
// Plugin Implementation
//------------------------------------------------------------------------
__PLUGIN_NAME__::__PLUGIN_NAME__()
: pluginProcess( nullptr )
, currentProcessMode( -1 ) // -1 means not initialized
{
    // register its editor class (the same as used in vstentry.cpp)
    setControllerClass( VST::PluginControllerUID );

    // should be created on setupProcessing, this however doesn't fire for Audio Unit using auval?
    pluginProcess = new PluginProcess( 2 );
}

//------------------------------------------------------------------------
__PLUGIN_NAME__::~__PLUGIN_NAME__()
{
    // free all allocated resources
    delete pluginProcess;
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::initialize( FUnknown* context )
{
    //---always initialize the parent-------
    tresult result = AudioEffect::initialize( context );
    // if everything Ok, continue
    if ( result != kResultOk )
        return result;

    //---create Audio In/Out buses------
    addAudioInput ( STR16( "Stereo In" ),  SpeakerArr::kStereo );
    addAudioOutput( STR16( "Stereo Out" ), SpeakerArr::kStereo );

    //---create Event In/Out buses (1 bus with only 1 channel)------
    addEventInput( STR16( "Event In" ), 1 );

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::terminate()
{
    // nothing to do here yet...except calling our parent terminate
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::setActive (TBool state)
{
    if (state)
        sendTextMessage( "__PLUGIN_NAME__::setActive (true)" );
    else
        sendTextMessage( "__PLUGIN_NAME__::setActive (false)" );

    // call our parent setActive
    return AudioEffect::setActive( state );
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::process( ProcessData& data )
{
    // In this example there are 4 steps:
    // 1) Read inputs parameters coming from host (in order to adapt our model values)
    // 2) Read inputs events coming from host (note on/off events)
    // 3) Apply the effect using the input buffer into the output buffer

    //---1) Read input parameter changes-----------
    IParameterChanges* paramChanges = data.inputParameterChanges;
    if ( paramChanges )
    {
        int32 numParamsChanged = paramChanges->getParameterCount();
        // for each parameter which are some changes in this audio block:
        for ( int32 i = 0; i < numParamsChanged; i++ )
        {
            IParamValueQueue* paramQueue = paramChanges->getParameterData( i );
            if ( paramQueue )
            {
                ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount();
                switch ( paramQueue->getParameterId())
                {
// --- AUTO-GENERATED PROCESS START


                    case kDelayTimeId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDelayTime = ( float ) value;
                        break;

                    case kDelayFeedbackId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDelayFeedback = ( float ) value;
                        break;

                    case kDelayMixId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDelayMix = ( float ) value;
                        break;

                    case kDelayHostSyncId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDelayHostSync = ( value > 0.5f );
                        break;

                    case kPitchShiftId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fPitchShift = ( float ) value;
                        break;

                    case kHarmonizeId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fHarmonize = ( float ) value;
                        break;

                    case kReverbId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fReverb = ( value > 0.5f );
                        break;

                    case kDecimatorId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fDecimator = ( float ) value;
                        break;

                    case kFilterCutoffId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFilterCutoff = ( float ) value;
                        break;

                    case kFilterResonanceId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fFilterResonance = ( float ) value;
                        break;

                    case kSyncChoirId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fSyncChoir = ( value > 0.5f );
                        break;

                    case kOddSpeedId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fOddSpeed = ( float ) value;
                        break;

                    case kEvenSpeedId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fEvenSpeed = ( float ) value;
                        break;

                    case kLinkGatesId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            fLinkGates = ( value > 0.5f );
                        break;

// --- AUTO-GENERATED PROCESS END

                    case kBypassId:
                        if ( paramQueue->getPoint( numPoints - 1, sampleOffset, value ) == kResultTrue )
                            _bypass = ( value > 0.5f );
                        break;
                }
                syncModel();
            }
        }
    }

    // according to docs: processing context (optional, but most welcome)

    if ( data.processContext != nullptr ) {
        // in case you want to do tempo synchronization with the host
        pluginProcess->setTempo(
            data.processContext->tempo, data.processContext->timeSigNumerator, data.processContext->timeSigDenominator
        );
    }

    //---2) Read input events-------------
//    IEventList* eventList = data.inputEvents;


    //-------------------------------------
    //---3) Process Audio---------------------
    //-------------------------------------

    if ( data.numInputs == 0 || data.numOutputs == 0 )
    {
        // nothing to do
        return kResultOk;
    }

    int32 numInChannels  = data.inputs[ 0 ].numChannels;
    int32 numOutChannels = data.outputs[ 0 ].numChannels;

    // --- get audio buffers----------------
    uint32 sampleFramesSize = getSampleFramesSizeInBytes( processSetup, data.numSamples );
    void** in  = getChannelBuffersPointer( processSetup, data.inputs [ 0 ] );
    void** out = getChannelBuffersPointer( processSetup, data.outputs[ 0 ] );

    // process the incoming sound!

    bool isDoublePrecision = ( data.symbolicSampleSize == kSample64 );

    if ( _bypass )
    {
        // bypass mode, write the input unchanged into the output
        for ( int32 i = 0, l = std::min( numInChannels, numOutChannels ); i < l; i++ )
		{
			if ( in[ i ] != out[ i ])
			{
				memcpy( out[ i ], in[ i ], sampleFramesSize );
			}
		}
    } else {
        // apply processing

        if ( isDoublePrecision ) {
            // 64-bit samples, e.g. Reaper64
            pluginProcess->process<double>(
                ( double** ) in, ( double** ) out, numInChannels, numOutChannels,
                data.numSamples, sampleFramesSize
            );
        }
        else {
            // 32-bit samples, e.g. Ableton Live, Bitwig Studio... (oddly enough also when 64-bit?)
            pluginProcess->process<float>(
                ( float** ) in, ( float** ) out, numInChannels, numOutChannels,
                data.numSamples, sampleFramesSize
            );
        }
    }

    // output flags

    data.outputs[ 0 ].silenceFlags = false; // there should always be output
    //float outputGain = pluginProcess->limiter->getLinearGR();

    return kResultOk;
}

//------------------------------------------------------------------------
tresult __PLUGIN_NAME__::receiveText( const char* text )
{
    // received from Controller
    fprintf( stderr, "[__PLUGIN_NAME__] received: " );
    fprintf( stderr, "%s", text );
    fprintf( stderr, "\n" );

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::setState( IBStream* state )
{
    // called when we load a preset, the model has to be reloaded

    IBStreamer streamer( state, kLittleEndian );

    int32 savedBypass = 0;
    if ( streamer.readInt32( savedBypass ) == false )
        return kResultFalse;

// --- AUTO-GENERATED SETSTATE START

    float savedDelayTime = 0.f;
    if ( streamer.readFloat( savedDelayTime ) == false )
        return kResultFalse;

    float savedDelayFeedback = 0.f;
    if ( streamer.readFloat( savedDelayFeedback ) == false )
        return kResultFalse;

    float savedDelayMix = 0.f;
    if ( streamer.readFloat( savedDelayMix ) == false )
        return kResultFalse;

    int32 savedDelayHostSync = 0;
    if ( streamer.readInt32( savedDelayHostSync ) == false )
        return kResultFalse;

    float savedPitchShift = 0.f;
    if ( streamer.readFloat( savedPitchShift ) == false )
        return kResultFalse;

    float savedHarmonize = 0.f;
    if ( streamer.readFloat( savedHarmonize ) == false )
        return kResultFalse;

    int32 savedReverb = 0;
    if ( streamer.readInt32( savedReverb ) == false )
        return kResultFalse;

    float savedDecimator = 0.f;
    if ( streamer.readFloat( savedDecimator ) == false )
        return kResultFalse;

    float savedFilterCutoff = 0.f;
    if ( streamer.readFloat( savedFilterCutoff ) == false )
        return kResultFalse;

    float savedFilterResonance = 0.f;
    if ( streamer.readFloat( savedFilterResonance ) == false )
        return kResultFalse;

    int32 savedSyncChoir = 0;
    if ( streamer.readInt32( savedSyncChoir ) == false )
        return kResultFalse;

    float savedOddSpeed = 0.f;
    if ( streamer.readFloat( savedOddSpeed ) == false )
        return kResultFalse;

    float savedEvenSpeed = 0.f;
    if ( streamer.readFloat( savedEvenSpeed ) == false )
        return kResultFalse;

    int32 savedLinkGates = 0;
    if ( streamer.readInt32( savedLinkGates ) == false )
        return kResultFalse;


// --- AUTO-GENERATED SETSTATE END

    _bypass = savedBypass > 0;

// --- AUTO-GENERATED SETSTATE APPLY START

    fDelayTime = savedDelayTime;
    fDelayFeedback = savedDelayFeedback;
    fDelayMix = savedDelayMix;
    fDelayHostSync = savedDelayHostSync > 0;
    fPitchShift = savedPitchShift;
    fHarmonize = savedHarmonize;
    fReverb = savedReverb > 0;
    fDecimator = savedDecimator;
    fFilterCutoff = savedFilterCutoff;
    fFilterResonance = savedFilterResonance;
    fSyncChoir = savedSyncChoir > 0;
    fOddSpeed = savedOddSpeed;
    fEvenSpeed = savedEvenSpeed;
    fLinkGates = savedLinkGates > 0;

// --- AUTO-GENERATED SETSTATE APPLY END

    syncModel();

    // Example of using the IStreamAttributes interface
    FUnknownPtr<IStreamAttributes> stream (state);
    if ( stream )
    {
        IAttributeList* list = stream->getAttributes ();
        if ( list )
        {
            // get the current type (project/Default..) of this state
            String128 string = {0};
            if ( list->getString( PresetAttributes::kStateType, string, 128 * sizeof( TChar )) == kResultTrue )
            {
                UString128 tmp( string );
                char ascii[ 128 ];
                tmp.toAscii( ascii, 128 );
                if ( !strncmp( ascii, StateType::kProject, strlen( StateType::kProject )))
                {
                    // we are in project loading context...
                }
            }

            // get the full file path of this state
            TChar fullPath[ 1024 ];
            memset( fullPath, 0, 1024 * sizeof( TChar ));
            if ( list->getString( PresetAttributes::kFilePathStringType,
                 fullPath, 1024 * sizeof( TChar )) == kResultTrue )
            {
                // here we have the full path ...
            }
        }
    }
    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::getState( IBStream* state )
{
    // here we save the model values

    IBStreamer streamer( state, kLittleEndian );

    streamer.writeInt32( _bypass ? 1 : 0 );

// --- AUTO-GENERATED GETSTATE START

    streamer.writeFloat( fDelayTime );
    streamer.writeFloat( fDelayFeedback );
    streamer.writeFloat( fDelayMix );
    streamer.writeInt32( fDelayHostSync ? 1 : 0 );
    streamer.writeFloat( fPitchShift );
    streamer.writeFloat( fHarmonize );
    streamer.writeInt32( fReverb ? 1 : 0 );
    streamer.writeFloat( fDecimator );
    streamer.writeFloat( fFilterCutoff );
    streamer.writeFloat( fFilterResonance );
    streamer.writeInt32( fSyncChoir ? 1 : 0 );
    streamer.writeFloat( fOddSpeed );
    streamer.writeFloat( fEvenSpeed );
    streamer.writeInt32( fLinkGates ? 1 : 0 );

// --- AUTO-GENERATED GETSTATE END

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::setupProcessing( ProcessSetup& newSetup )
{
    // called before the process call, always in a disabled state (not active)

    // here we keep a trace of the processing mode (offline,...) for example.
    currentProcessMode = newSetup.processMode;

    VST::SAMPLE_RATE = newSetup.sampleRate;

    // spotted to fire multiple times...

    if ( pluginProcess != nullptr )
        delete pluginProcess;

    // TODO: creating a bunch of extra channels for no apparent reason?
    // get the correct channel amount and don't allocate more than necessary...
    pluginProcess = new PluginProcess( 6 );

    syncModel();

    return AudioEffect::setupProcessing( newSetup );
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::setBusArrangements( SpeakerArrangement* inputs,  int32 numIns,
                                                 SpeakerArrangement* outputs, int32 numOuts )
{
    if ( numIns == 1 && numOuts == 1 )
    {
        // the host wants Mono => Mono (or 1 channel -> 1 channel)
        if ( SpeakerArr::getChannelCount( inputs[0])  == 1 &&
             SpeakerArr::getChannelCount( outputs[0]) == 1 )
        {
            AudioBus* bus = FCast<AudioBus>( audioInputs.at( 0 ));
            if ( bus )
            {
                // check if we are Mono => Mono, if not we need to recreate the buses
                if ( bus->getArrangement() != inputs[0])
                {
                    removeAudioBusses();
                    addAudioInput ( STR16( "Mono In" ),  inputs[0] );
                    addAudioOutput( STR16( "Mono Out" ), inputs[0] );
                }
                return kResultOk;
            }
        }
        // the host wants something else than Mono => Mono, in this case we are always Stereo => Stereo
        else
        {
            AudioBus* bus = FCast<AudioBus>( audioInputs.at(0));
            if ( bus )
            {
                tresult result = kResultFalse;

                // the host wants 2->2 (could be LsRs -> LsRs)
                if ( SpeakerArr::getChannelCount(inputs[0]) == 2 && SpeakerArr::getChannelCount( outputs[0]) == 2 )
                {
                    removeAudioBusses();
                    addAudioInput  ( STR16( "Stereo In"),  inputs[0] );
                    addAudioOutput ( STR16( "Stereo Out"), outputs[0]);
                    result = kResultTrue;
                }
                // the host want something different than 1->1 or 2->2 : in this case we want stereo
                else if ( bus->getArrangement() != SpeakerArr::kStereo )
                {
                    removeAudioBusses();
                    addAudioInput ( STR16( "Stereo In"),  SpeakerArr::kStereo );
                    addAudioOutput( STR16( "Stereo Out"), SpeakerArr::kStereo );
                    result = kResultFalse;
                }
                return result;
            }
        }
    }
    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::canProcessSampleSize( int32 symbolicSampleSize )
{
    if ( symbolicSampleSize == kSample32 )
        return kResultTrue;

    // we support double processing
    if ( symbolicSampleSize == kSample64 )
        return kResultTrue;

    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API __PLUGIN_NAME__::notify( IMessage* message )
{
    if ( !message )
        return kInvalidArgument;

    if ( !strcmp( message->getMessageID(), "BinaryMessage" ))
    {
        const void* data;
        uint32 size;
        if ( message->getAttributes ()->getBinary( "MyData", data, size ) == kResultOk )
        {
            // we are in UI thread
            // size should be 100
            if ( size == 100 && ((char*)data)[1] == 1 ) // yeah...
            {
                fprintf( stderr, "[__PLUGIN_NAME__] received the binary message!\n" );
            }
            return kResultOk;
        }
    }

    return AudioEffect::notify( message );
}

void __PLUGIN_NAME__::syncModel()
{
    // forward the protected model values onto the plugin process and related processors
    // NOTE: when dealing with "bool"-types, use Calc::toBool() to determine on/off

    pluginProcess->syncDelayToHost = Calc::toBool( fDelayHostSync );
    pluginProcess->setDelayTime( fDelayTime );
    pluginProcess->setDelayFeedback( fDelayFeedback );
    pluginProcess->setDelayMix( fDelayMix );

    // a pitch shift of 0.5 is neutral (no shift), < 0.5 is shift down, > 0.5 is shift up

    bool isShiftUp   = fPitchShift >= PitchShifter::UNCHANGED;
    float shiftValue = isShiftUp ? Calc::scale( fPitchShift, 1.f, PitchShifter::OCTAVE_UP ) : fPitchShift + PitchShifter::OCTAVE_DOWN;

    pluginProcess->setPitchShift( shiftValue );
    pluginProcess->setHarmony( fHarmonize, Calc::toBool( fSyncChoir ));
    pluginProcess->setHarmonyStepSpeed( fOddSpeed, fEvenSpeed, Calc::toBool( fLinkGates ));

    pluginProcess->decimator->setRate( fDecimator > 0.99f ? 0.49f : Calc::inverseNormalize( fDecimator ) * 0.5f );
    pluginProcess->filter->updateProperties( fFilterCutoff, Calc::inverseNormalize( fFilterResonance ));
    pluginProcess->enableReverb( Calc::toBool( fReverb ));
}

}
