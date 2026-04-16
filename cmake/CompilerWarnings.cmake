# ============================================================================
# Compiler warnings (per-target)
# ============================================================================
function(set_clap_compiler_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX /analyze /wd4996)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
            -Wcast-align
            -Wwrite-strings
            -Wstrict-prototypes
            -Wmissing-prototypes
            -Wmissing-declarations
            -Wredundant-decls
            -Wnested-externs
            -Wno-long-long
            -Wno-overlength-strings
        )
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()