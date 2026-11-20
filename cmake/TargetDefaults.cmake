# rcs_configure_target(<target>): the settings every target of ours gets — warnings, and
# coverage instrumentation when RCS_COVERAGE is on. Dependencies never get these.

set(RCS_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wold-style-cast
    -Wcast-align
    -Wdouble-promotion
    -Wimplicit-fallthrough)

if(RCS_COVERAGE AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "RCS_COVERAGE uses clang's source-based coverage; build with clang.")
endif()

function(rcs_configure_target target)
    target_compile_options(${target} PRIVATE ${RCS_WARNING_FLAGS})
    if(RCS_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE -Werror)
    endif()

    if(RCS_COVERAGE)
        target_compile_options(${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${target} PRIVATE -fprofile-instr-generate)
    endif()
endfunction()
