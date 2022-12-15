cmake_minimum_required(VERSION 3.19)

option(JAMBA_ENABLE_VST2 "Use VST2" OFF)
option(JAMBA_ENABLE_AUDIO_UNIT "Enable Audio Unit" ON)
option(JAMBA_DOWNLOAD_VSTSDK "Download VST SDK if not installed" OFF)
set(JAMBA_MACOS_DEPLOYMENT_TARGET "10.10" CACHE STRING "macOS deployment target")

# To use local jamba install, uncomment the following line (no download)
#set(JAMBA_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/build/jamba")

# download jamba framework
include("fetch_jamba.cmake")

# Determine proper architecture for the project
include("${JAMBA_ROOT_DIR}/cmake/JambaSetArchitecture.cmake")

# Plugin version
set(PLUGIN_MAJOR_VERSION 1)
set(PLUGIN_MINOR_VERSION 0)
set(PLUGIN_PATCH_VERSION 0)
set(PLUGIN_VERSION "${major_version}.${minor_version}.${release_number}")

# Project
project("${target}" VERSION "${PLUGIN_VERSION}")

# To use local googletest install, uncomment the following line (no download) and modify the path accordingly
set(GOOGLETEST_ROOT_DIR "")
#set(GOOGLETEST_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../google/googletest")

# Include Jamba
include("${JAMBA_ROOT_DIR}/jamba.cmake")

# Generating the version.h header file which contains the plugin version (to make sure it is in sync with the version
# defined here)
set(VERSION_DIR "${CMAKE_BINARY_DIR}/generated")

# Location of resources
# set(RES_DIR "${CMAKE_CURRENT_LIST_DIR}/resource")

# List of test cases
set(test_case_sources
#    "${TEST_DIR}/test-rechoir.cpp"
)
