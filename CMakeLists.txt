cmake_minimum_required(VERSION 3.28)

project(binhound
    VERSION 0.1.0
    DESCRIPTION "Static SBOM/CBOM generator and CRA readiness checker for compiled binaries"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(BINHOUND_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(BINHOUND_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(BINHOUND_ENABLE_LTO "Enable link-time optimization" OFF)
option(BINHOUND_ENABLE_HARDENING "Enable security hardening flags" OFF)
option(BINHOUND_ENABLE_NATIVE "Optimize for the local CPU only (never distribute)" OFF)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

include(FetchContent)

set(EXPECTED_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(EXPECTED_BUILD_PACKAGE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(tl_expected
    GIT_REPOSITORY https://github.com/TartanLlama/expected.git
    GIT_TAG v1.3.1
    SYSTEM
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(tl_expected)

set(DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "" FORCE)
set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.5.3
    SYSTEM
    EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(doctest)

add_library(binhound_options INTERFACE)

if(MSVC)
    target_compile_options(binhound_options INTERFACE /W4 /permissive-)
    if(BINHOUND_WARNINGS_AS_ERRORS)
        target_compile_options(binhound_options INTERFACE /WX)
    endif()
else()
    target_compile_options(binhound_options INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-qual
        -Wformat=2
        -Wundef)

    if(BINHOUND_WARNINGS_AS_ERRORS)
        target_compile_options(binhound_options INTERFACE -Werror)
    endif()

    if(BINHOUND_ENABLE_SANITIZERS)
        target_compile_options(binhound_options INTERFACE
            -fsanitize=address,undefined
            -fno-sanitize-recover=all
            -fno-omit-frame-pointer)
        target_link_options(binhound_options INTERFACE -fsanitize=address,undefined)
    endif()

    if(BINHOUND_ENABLE_NATIVE)
        target_compile_options(binhound_options INTERFACE -march=native)
    endif()

    if(BINHOUND_ENABLE_HARDENING)
        include(CheckCXXCompilerFlag)
        foreach(flag IN ITEMS -fstack-protector-strong -fstack-clash-protection -fcf-protection)
            string(MAKE_C_IDENTIFIER "BINHOUND_HAS${flag}" flag_id)
            check_cxx_compiler_flag("${flag}" ${flag_id})
            if(${flag_id})
                target_compile_options(binhound_options INTERFACE "${flag}")
            endif()
        endforeach()
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            target_compile_definitions(binhound_options INTERFACE _FORTIFY_SOURCE=3)
            target_link_options(binhound_options INTERFACE -Wl,-z,relro -Wl,-z,now)
        endif()
    endif()
endif()

add_library(binhound_core STATIC
    src/util/error.cpp)

target_sources(binhound_core
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS src
        FILES
            src/util/error.hpp)

target_include_directories(binhound_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_compile_definitions(binhound_core PUBLIC
    BINHOUND_VERSION="${PROJECT_VERSION}")

target_link_libraries(binhound_core
    PUBLIC
        tl::expected
        binhound_options)

add_executable(binhound
    src/cli/main.cpp)

target_link_libraries(binhound PRIVATE binhound_core)

if(BINHOUND_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT BINHOUND_IPO_SUPPORTED OUTPUT BINHOUND_IPO_OUTPUT)
    if(BINHOUND_IPO_SUPPORTED)
        set_property(TARGET binhound_core PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET binhound PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "LTO not supported by the compiler: ${BINHOUND_IPO_OUTPUT}")
    endif()
endif()

find_program(CLANG_FORMAT_PROGRAM clang-format)
if(CLANG_FORMAT_PROGRAM)
    add_custom_target(format
        COMMAND "${CLANG_FORMAT_PROGRAM}" -i
            ${CMAKE_CURRENT_SOURCE_DIR}/src/util/error.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/util/error.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/cli/main.cpp
        COMMENT "Running clang-format"
        VERBATIM)
endif()

include(GNUInstallDirs)
install(TARGETS binhound RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/signatures/components.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/binhound
    OPTIONAL)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
