cmake_minimum_required(VERSION 3.20)

project(binhound
    VERSION 0.1.0
    DESCRIPTION "Static SBOM/CBOM generator and CRA readiness checker for compiled binaries"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BINHOUND_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(BINHOUND_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(BINHOUND_ENABLE_LTO "Enable link-time optimization" OFF)
option(BINHOUND_ENABLE_HARDENING "Enable security hardening flags" OFF)
option(BINHOUND_ENABLE_NATIVE "Optimize for the local CPU only (never distribute)" OFF)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

add_executable(binhound
    src/cli/main.cpp
)

target_compile_definitions(binhound PRIVATE
    BINHOUND_VERSION="${PROJECT_VERSION}"
)

if(MSVC)
    target_compile_options(binhound PRIVATE /W4 /permissive-)
    if(BINHOUND_WARNINGS_AS_ERRORS)
        target_compile_options(binhound PRIVATE /WX)
    endif()
else()
    target_compile_options(binhound PRIVATE
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
        target_compile_options(binhound PRIVATE -Werror)
    endif()

    if(BINHOUND_ENABLE_SANITIZERS)
        target_compile_options(binhound PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(binhound PRIVATE -fsanitize=address,undefined)
    endif()

    if(BINHOUND_ENABLE_NATIVE)
        target_compile_options(binhound PRIVATE -march=native)
    endif()

    if(BINHOUND_ENABLE_HARDENING)
        target_compile_options(binhound PRIVATE
            -fstack-protector-strong
            -fstack-clash-protection
            -fcf-protection)
        target_link_options(binhound PRIVATE
            -Wl,-z,relro
            -Wl,-z,now)
        if(CMAKE_BUILD_TYPE STREQUAL "Release"
           OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"
           OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
            target_compile_definitions(binhound PRIVATE _FORTIFY_SOURCE=3)
        endif()
    endif()
endif()

if(BINHOUND_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT BINHOUND_IPO_SUPPORTED OUTPUT BINHOUND_IPO_OUTPUT)
    if(BINHOUND_IPO_SUPPORTED)
        set_property(TARGET binhound PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "LTO not supported by the compiler: ${BINHOUND_IPO_OUTPUT}")
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
