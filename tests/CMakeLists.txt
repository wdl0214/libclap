# ============================================================================
# Fetch Unity Test Framework
# ============================================================================
include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.5.2
)

FetchContent_MakeAvailable(unity)

# ============================================================================
# Common test configuration
# ============================================================================
function(add_clap_test test_name test_source)
    add_executable(${test_name} ${test_source} ${ARGN})
    
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
# Combined Test Runner
# ============================================================================

# set(COMBINED_TEST_SOURCES
#     test_main.c
#     unit/test_clap.c
#     unit/test_allocator.c
#     unit/test_arena.c
#     unit/test_buffer.c
#     unit/test_trie.c
#     unit/test_error.c
#     unit/test_parser.c
#     unit/test_argument.c
#     unit/test_namespace.c
#     unit/test_validator.c
#     unit/test_convert.c
#     unit/test_actions.c
#     unit/test_mutex.c
#     unit/test_dependency.c
#     unit/test_subparser.c
#     unit/test_find.c
#     unit/test_formatter.c
#     unit/test_tokenizer.c
# )

# add_executable(test_all ${COMBINED_TEST_SOURCES})
# target_link_libraries(test_all PRIVATE clap_static unity)
# target_include_directories(test_all PRIVATE 
#     ${unity_SOURCE_DIR}/src
#     ${CMAKE_SOURCE_DIR}/src
# )
# add_test(NAME test_all COMMAND test_all)

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