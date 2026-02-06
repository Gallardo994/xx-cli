cmake_minimum_required(VERSION 3.22)

project(xx-root LANGUAGES CXX)

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
        add_compile_options(/EHsc)
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

add_subdirectory(xx-lib)
add_subdirectory(xx)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
