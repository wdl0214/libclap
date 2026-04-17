# ============================================================================
# Fetch Unity Test Framework
# ============================================================================
include(FetchContent)

set(UNITY_INCLUDE_DOUBLE ON CACHE BOOL "Enable double precision support in Unity" FORCE)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.5.2
)
cmake_policy(SET CMP0169 OLD)

FetchContent_GetProperties(unity)
if(NOT unity_POPULATED)
    FetchContent_Populate(unity)
    add_subdirectory(${unity_SOURCE_DIR} ${unity_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

cmake_policy(SET CMP0169 NEW)

# ============================================================================
# Common test configuration
# ============================================================================
function(add_clap_test test_name test_source)
    add_executable(${test_name} ${test_source} ${ARGN})
    target_compile_definitions(${test_name} PRIVATE -DUNITY_INCLUDE_DOUBLE)
    target_compile_definitions(${test_name} PRIVATE -DSTANDALONE_TEST)
    
    target_link_libraries(${test_name} PRIVATE clap_static unity)
    target_include_directories(${test_name} PRIVATE 
        ${unity_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src
    )
    
    if(MINGW)
        set_target_properties(${test_name} PROPERTIES WIN32_EXECUTABLE OFF)
        target_link_options(${test_name} PRIVATE -mconsole)
    endif()
    
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endfunction()

# ============================================================================
# Unit Tests
# ============================================================================
set(UNIT_TESTS
    test_clap
    test_allocator
    test_arena
    test_buffer
    test_trie
    test_error
    test_parser
    test_argument
    test_namespace
    test_validator
    test_convert
    test_actions
    test_mutex
    test_dependency
    test_subparser
    test_find
    test_formatter
    test_tokenizer
)

foreach(test ${UNIT_TESTS})
    add_clap_test(${test} unit/${test}.c)
endforeach()

# ============================================================================
# Integration Tests
# ============================================================================
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration/test_usage.c)
    add_clap_test(test_usage integration/test_usage.c)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration/test_parse_flow.c)
    add_clap_test(test_parse_flow integration/test_parse_flow.c)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/integration/test_scenarios.c)
    add_clap_test(test_scenarios integration/test_scenarios.c)
endif()

# ============================================================================
# Platform-specific test adjustments
# ============================================================================
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_tests_properties(test_error PROPERTIES DISABLED TRUE)
    message(STATUS "Disabled test_error on Windows Debug due to timeout issues")
endif()

# ============================================================================
# Fuzz Testing (optional)
# ============================================================================
option(CLAP_BUILD_FUZZ "Build fuzz test" OFF)
if(CLAP_BUILD_FUZZ AND CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_executable(fuzz_clap fuzz/fuzz_clap.c)
    target_compile_options(fuzz_clap PRIVATE -fsanitize=fuzzer,address)
    target_link_options(fuzz_clap PRIVATE -fsanitize=fuzzer,address)
    target_link_libraries(fuzz_clap PRIVATE clap_static)
    
    # Create corpus directory
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/corpus)
    
    # Add fuzz test
    add_test(NAME fuzz_clap 
        COMMAND fuzz_clap 
            -max_len=4096 
            -runs=100000 
            ${CMAKE_CURRENT_BINARY_DIR}/corpus)
    set_tests_properties(fuzz_clap PROPERTIES
        RUN_SERIAL TRUE
        TIMEOUT 300)
endif()
