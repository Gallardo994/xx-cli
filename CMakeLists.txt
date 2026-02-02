cmake_minimum_required(VERSION 3.22)

project(xx-root VERSION 0.6.0 LANGUAGES CXX)
include(FetchContent)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

if (ENABLE_SANITIZERS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if (WIN32)
            message(WARNING "Sanitizers are not supported on Windows for GCC/Clang (yet)")
        else()
            string(APPEND CMAKE_CXX_FLAGS " -fsanitize=address,undefined -fno-omit-frame-pointer")
            string(APPEND CMAKE_LINKER_FLAGS " -fsanitize=address,undefined")
            message(STATUS "Sanitizers enabled for GCC/Clang")
        endif()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(WARNING "Sanitizers are not supported for MSVC")
    endif()
else()
    message(STATUS "Sanitizers disabled")
endif()

if (WIN32)
    if(MSVC)
        message(STATUS "Using MSVC compiler")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        message(STATUS "Using non-MSVC compiler")
        string(APPEND CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    endif()
endif()

if (APPLE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        string(APPEND CMAKE_EXE_LINKER_FLAGS " -pthread")
        message(STATUS "Added -pthread to linker flags for Apple platforms")
    endif()
endif()

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(xx-lib)
add_subdirectory(xx)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
