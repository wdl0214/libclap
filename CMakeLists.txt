cmake_minimum_required(VERSION 3.15)
project(clap VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Options
# ============================================================================
option(CLAP_BUILD_SHARED "Build shared library" ON)
option(CLAP_BUILD_STATIC "Build static library" ON)
option(CLAP_BUILD_TESTS "Build unit tests" ON)
option(CLAP_BUILD_EXAMPLES "Build examples" ON)
option(CLAP_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(CLAP_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(CLAP_ENABLE_COVERAGE "Enable code coverage" OFF)

# ============================================================================
# CMake module path
# ============================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ============================================================================
# Include custom CMake modules
# ============================================================================
include(CompilerWarnings)
include(Sanitizers)

include(GNUInstallDirs)

# ============================================================================
# Code coverage (per-target)
# ============================================================================
function(set_clap_coverage target)
    if(CLAP_ENABLE_COVERAGE AND CMAKE_COMPILER_IS_GNUCC)
        target_compile_options(${target} PRIVATE --coverage)
        target_link_options(${target} PRIVATE --coverage)
    endif()
endfunction()

# ============================================================================
# Platform definitions (per-target)
# ============================================================================
function(set_clap_platform_definitions target)
    if(UNIX AND NOT APPLE)
        target_compile_definitions(${target} PRIVATE
            _POSIX_C_SOURCE=200809L
            _XOPEN_SOURCE=700
        )
    endif()

    if(APPLE)
        target_compile_definitions(${target} PRIVATE
            _DARWIN_C_SOURCE
        )
    endif()

    if(WIN32)
        target_compile_definitions(${target} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            _WIN32_WINNT=0x0601
        )
    endif()
endfunction()

# ============================================================================
# Subdirectories
# ============================================================================
add_subdirectory(src)

if(CLAP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CLAP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================================
# Headers installation
# ============================================================================
install(
    DIRECTORY include/clap
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# ============================================================================
# CMake package configuration
# ============================================================================
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/clapConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clapConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/clapConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clap
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/clapConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/clapConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clap
)

install(
    EXPORT clapTargets
    NAMESPACE clap::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clap
)

# ============================================================================
# pkg-config file
# ============================================================================
if(UNIX)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libclap.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/libclap.pc"
        @ONLY
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/libclap.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
endif()

# ============================================================================
# Coverage target
# ============================================================================
if(CLAP_ENABLE_COVERAGE AND CMAKE_COMPILER_IS_GNUCC)
    add_custom_target(coverage
        COMMAND lcov --capture --directory . --output-file coverage.info
        COMMAND lcov --remove coverage.info '/usr/*' '*/tests/*' '*/_deps/*' --output-file coverage.info
        COMMAND genhtml coverage.info --output-directory coverage
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating code coverage report"
    )
endif()