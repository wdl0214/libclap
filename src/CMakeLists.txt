# ============================================================================
# Source files
# ============================================================================
set(CLAP_SOURCES
    clap.c
    clap_parser.c
    clap_argument.c
    clap_namespace.c
    clap_error.c
    clap_tokenizer.c
    clap_validator.c
    clap_actions.c
    clap_mutex.c
    clap_dependency.c
    clap_convert.c
    clap_formatter.c
    clap_subparser.c
    clap_find.c
    clap_buffer.c
    clap_arena.c
    clap_trie.c
    clap_allocator.c
)

# ============================================================================
# Static library
# ============================================================================
if(CLAP_BUILD_STATIC)
    add_library(clap_static STATIC ${CLAP_SOURCES})
    set_target_properties(clap_static PROPERTIES
        OUTPUT_NAME clap
        POSITION_INDEPENDENT_CODE ON
    )
    target_include_directories(clap_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_clap_compiler_warnings(clap_static)
    set_clap_sanitizers(clap_static)
    set_clap_coverage(clap_static)
    set_clap_platform_definitions(clap_static)
    
    install(
        TARGETS clap_static
        EXPORT clapTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# ============================================================================
# Shared library
# ============================================================================
if(CLAP_BUILD_SHARED)
    add_library(clap_shared SHARED ${CLAP_SOURCES})
    set_target_properties(clap_shared PROPERTIES
        OUTPUT_NAME clap
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        C_VISIBILITY_PRESET hidden
    )
    target_include_directories(clap_shared
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_clap_compiler_warnings(clap_shared)
    set_clap_sanitizers(clap_shared)
    set_clap_coverage(clap_shared)
    set_clap_platform_definitions(clap_shared)
    
    if(UNIX)
        target_link_libraries(clap_shared PRIVATE m)
    endif()
    
    install(
        TARGETS clap_shared
        EXPORT clapTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()