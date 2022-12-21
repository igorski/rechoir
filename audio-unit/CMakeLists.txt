
cmake_minimum_required(VERSION 3.15.0)

project(smtg-vst3-rechoir
    VERSION ${vstsdk_VERSION}.0
    DESCRIPTION "Steinberg VST 3 rechoir example"
)

if(NOT SMTG_ADD_VSTGUI)
    return()
endif()

if (SMTG_CREATE_VST2_rechoir_SAMPLE_VERSION)
    set(rechoir_vst2_sources
        ${SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.sdk.cpp
        source/rechoirentry_vst2.cpp
    )
endif(SMTG_CREATE_VST2_rechoir_SAMPLE_VERSION)

set(rechoir_sources
    src/global.h
    src/allpass.h
    src/allpass.cpp
    src/audiobuffer.h
    src/audiobuffer.cpp
    src/comb.h
    src/comb.cpp
    src/decimator.h
    src/decimator.cpp
    src/filter.h
    src/filter.cpp
    src/limiter.h
    src/limiter.cpp
    src/paramids.h
    src/pitchshifter.h
    src/pitchshifter.cpp
    src/plugin_process.h
    src/plugin_process.cpp
    src/reverb.h
    src/reverb.cpp
    src/tablepool.h
    src/tablepool.cpp
    src/vst.h
    src/vst.cpp
    src/vstentry.cpp
    src/version.h
    src/waveforms.h
    src/wavegenerator.h
    src/wavegenerator.cpp
    src/wavetable.h
    src/wavetable.cpp
    src/ui/controller.h
    src/ui/controller.cpp
    src/ui/uimessagecontroller.h
    resource/plugin.uidesc
)
set(target rechoir)

smtg_add_vst3plugin(${target} ${rechoir_sources})
smtg_target_configure_version_file(${target})

#smtg_add_vst3plugin(${target} PACKAGE_NAME "Rechoir" SOURCES_LIST ${rechoir_sources})
set_target_properties(${target}
    PROPERTIES
        ${SDK_IDE_PLUGIN_EXAMPLES_FOLDER}
)

target_compile_features(${target}
    PUBLIC
        cxx_std_17
)

target_link_libraries(${target}
    PRIVATE
        sdk
        vstgui_support
)
smtg_target_add_plugin_resources(${target}
    RESOURCES
        resource/plugin.uidesc
        resource/background.png
        resource/version.png
)

#smtg_target_add_plugin_snapshots (${target}
#    RESOURCES
#        resource/84E8DE5F92554F5396FAE4133C935A18_snapshot.png
#        resource/84E8DE5F92554F5396FAE4133C935A18_snapshot_2.0x.png
#        resource/41347FD6FED64094AFBB12B7DBA1D441_snapshot.png
#        resource/41347FD6FED64094AFBB12B7DBA1D441_snapshot_2.0x.png
#)

if(SMTG_MAC)
    smtg_target_set_bundle(${target}
        BUNDLE_IDENTIFIER "nl.igorski.vst.${target}"
        COMPANY_NAME "igorski.nl"
    )
elseif(SMTG_WIN)
    target_sources(${target}
        PRIVATE
            resource/plugin.rc
    )
endif(SMTG_MAC)

# Add an AUv2 target
if (SMTG_MAC AND XCODE AND SMTG_COREAUDIO_SDK_PATH)
	include(SMTG_AddVST3AuV2)
	smtg_target_add_auv2(rechoir-au
		BUNDLE_NAME rechoir
		BUNDLE_IDENTIFIER nl.igorski.vst.${target}.audiounit
		INFO_PLIST_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/resource/au-info.plist
		VST3_PLUGIN_TARGET rechoir)
endif(SMTG_MAC AND XCODE AND SMTG_COREAUDIO_SDK_PATH)
