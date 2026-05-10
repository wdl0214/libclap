cmake_minimum_required(VERSION 3.15)
project(clap VERSION 1.1.0 LANGUAGES C)

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
    if(CLAP_ENABLE_COVERAGE)
        if(CMAKE_COMPILER_IS_GNUCC)
            target_compile_options(${target} PRIVATE --coverage)
            target_link_options(${target} PRIVATE --coverage)
            target_link_libraries(${target} PRIVATE gcov)
        elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
            target_compile_options(${target} PRIVATE
                -fprofile-instr-generate
                -fcoverage-mapping
            )
            target_link_options(${target} PRIVATE
                -fprofile-instr-generate
                -fcoverage-mapping
            )
        endif()
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
# i18n: install .pot template so package users can find translatable strings
# ============================================================================
install(
    FILES po/libclap.pot
    DESTINATION ${CMAKE_INSTALL_DATADIR}/libclap
    OPTIONAL
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
if(UNIX OR MINGW)
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
if(CLAP_ENABLE_COVERAGE)
    find_program(GCOVR gcovr)

    if(GCOVR)
        add_custom_target(coverage
            COMMAND ${GCOVR} -r ${CMAKE_SOURCE_DIR}
                    --html --html-details -o coverage.html
            COMMAND ${GCOVR} -r ${CMAKE_SOURCE_DIR}
                    --xml -o coverage.xml
            COMMAND ${GCOVR} -r ${CMAKE_SOURCE_DIR}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report with gcovr"
        )
        message(STATUS "Coverage enabled: using gcovr")
    else()
        message(WARNING "gcovr not found. Install with: pip install gcovr")
    endif()
endif()

# ============================================================================
# Documentation target (optional, requires Doxygen)
# ============================================================================
find_program(DOXYGEN doxygen)
if(DOXYGEN)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        @ONLY
    )
    add_custom_target(docs
        COMMAND ${DOXYGEN} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
    message(STATUS "Doxygen found: use 'make docs' to generate API docs")
endif()

# ============================================================================
# i18n: po/libclap.pot generation (optional, requires xgettext)
# ============================================================================
find_program(XGETTEXT xgettext)
if(XGETTEXT)
    file(GLOB CLAP_I18N_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
    add_custom_target(pot
        COMMAND ${XGETTEXT} -kCLAP_TR
            --language=C
            --from-code=UTF-8
            --package-name=libclap
            --package-version=${PROJECT_VERSION}
            --msgid-bugs-address=https://github.com/wdl0214/libclap
            --add-comments=TRANSLATORS:
            -o ${CMAKE_CURRENT_SOURCE_DIR}/po/libclap.pot
            ${CLAP_I18N_SOURCES}
        COMMENT "Regenerating po/libclap.pot translation template"
    )
    message(STATUS "xgettext found: use 'make pot' to regenerate po/libclap.pot")
endif()
