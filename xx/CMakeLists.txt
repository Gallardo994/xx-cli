cmake_minimum_required(VERSION 3.22)

project(xx VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/detail/command.cpp
    src/detail/parser.cpp
    src/detail/planner.cpp
    src/detail/platform.cpp
    src/main.cpp
)

if (WIN32)
    message(STATUS "Configuring for Windows platform")
    list(APPEND SOURCES src/detail/executor_windows.cpp)
    if(MSVC)
        message(STATUS "Using MSVC compiler")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        message(STATUS "Using non-MSVC compiler")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
elseif(UNIX)
    message(STATUS "Configuring for UNIX platform")
    list(APPEND SOURCES src/detail/executor_unix.cpp)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

add_executable(xx ${SOURCES})
target_compile_features(xx PRIVATE cxx_std_23)
target_include_directories(xx PRIVATE include)
