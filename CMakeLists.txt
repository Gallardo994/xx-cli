cmake_minimum_required(VERSION 3.22)

project(xx-root VERSION 0.7.0 LANGUAGES CXX)
include(FetchContent)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

if (ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT WIN32)
    string(APPEND CMAKE_CXX_FLAGS " -fsanitize=address,undefined -fno-omit-frame-pointer")
    string(APPEND CMAKE_LINKER_FLAGS " -fsanitize=address,undefined")
    message(STATUS "Sanitizers enabled")
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
        # MacOS + gcc results into linker errors for threads: https://stackoverflow.com/questions/54587052/cmake-on-mac-could-not-find-threads-missing-threads-found
        set(CMAKE_THREAD_LIBS_INIT "-lpthread")
        set(CMAKE_HAVE_THREADS_LIBRARY 1)
        set(CMAKE_USE_WIN32_THREADS_INIT 0)
        set(CMAKE_USE_PTHREADS_INIT 1)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
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
