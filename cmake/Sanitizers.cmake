# Global sanitizer flags, from RCS_SANITIZERS (a list such as "address;undefined" or "thread").
# Applied with add_compile_options so every target created afterwards, including fetched
# dependencies, is instrumented: mixing instrumented and plain code gives false reports.

if(NOT RCS_SANITIZERS)
    return()
endif()

if("address" IN_LIST RCS_SANITIZERS AND "thread" IN_LIST RCS_SANITIZERS)
    message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer cannot be combined; use two presets.")
endif()

list(JOIN RCS_SANITIZERS "," _rcs_sanitize)
message(STATUS "Sanitizers: ${_rcs_sanitize}")

add_compile_options(
    -fsanitize=${_rcs_sanitize}
    -fno-omit-frame-pointer
    # Stop at the first report instead of printing it and carrying on, so CI fails.
    -fno-sanitize-recover=all)
add_link_options(-fsanitize=${_rcs_sanitize})
