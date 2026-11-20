# Blocker B-1 (docs/TECHNICAL_DECISIONS.md): a newer macOS SDK can be installed next to older
# Command Line Tools whose linker cannot read it, so every link fails with
# "tapi error: malformed file ... unknown architecture".
#
# On the first configure, link a tiny program with the default SDK. If that fails, try the
# installed SDKs from newest to oldest and use the first one that links. The choice is cached in
# CMAKE_OSX_SYSROOT, so the check runs once per build directory. Setting CMAKE_OSX_SYSROOT or
# SDKROOT yourself skips it. Once the tools are updated, this does nothing.

if(NOT CMAKE_HOST_APPLE OR CMAKE_OSX_SYSROOT OR DEFINED ENV{SDKROOT})
    return()
endif()

set(_rcs_probe_dir "${CMAKE_BINARY_DIR}/CMakeFiles/rcs-sdk-probe")
file(WRITE "${_rcs_probe_dir}/probe.cpp" "int main() { return 0; }\n")

function(_rcs_sdk_links out_var)
    execute_process(
        COMMAND xcrun clang++ ${ARGN} "${_rcs_probe_dir}/probe.cpp" -o "${_rcs_probe_dir}/probe"
        RESULT_VARIABLE result
        OUTPUT_QUIET ERROR_QUIET)
    if(result EQUAL 0)
        set(${out_var} TRUE PARENT_SCOPE)
    else()
        set(${out_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

_rcs_sdk_links(_rcs_default_ok)
if(_rcs_default_ok)
    return()
endif()

file(GLOB _rcs_sdks LIST_DIRECTORIES true "/Library/Developer/CommandLineTools/SDKs/MacOSX*.*.sdk")
list(SORT _rcs_sdks COMPARE NATURAL ORDER DESCENDING)

foreach(_rcs_sdk IN LISTS _rcs_sdks)
    _rcs_sdk_links(_rcs_ok -isysroot "${_rcs_sdk}")
    if(_rcs_ok)
        message(STATUS "Default macOS SDK cannot link (blocker B-1); using ${_rcs_sdk}")
        set(CMAKE_OSX_SYSROOT "${_rcs_sdk}" CACHE PATH "macOS SDK used for this build" FORCE)
        return()
    endif()
endforeach()

message(WARNING "No installed macOS SDK can link a test program. Update the Command Line Tools "
                "(xcode-select --install) or set SDKROOT. See docs/TECHNICAL_DECISIONS.md B-1.")
