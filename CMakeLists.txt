cmake_minimum_required(VERSION 3.20)

project(binhound
    VERSION 0.1.0
    DESCRIPTION "Static SBOM/CBOM generator and CRA readiness checker for compiled binaries"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BINHOUND_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(BINHOUND_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

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
    target_compile_options(binhound PRIVATE -Wall -Wextra -Wpedantic)
    if(BINHOUND_WARNINGS_AS_ERRORS)
        target_compile_options(binhound PRIVATE -Werror)
    endif()
    if(BINHOUND_ENABLE_SANITIZERS)
        target_compile_options(binhound PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(binhound PRIVATE -fsanitize=address,undefined)
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
