cmake_minimum_required(VERSION 3.22)

project(xx-root VERSION 0.6.0 LANGUAGES CXX)
include(FetchContent)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

if (ENABLE_SANITIZERS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
        set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address,undefined")

        message(STATUS "Sanitizers enabled for GNU/Clang")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(WARNING "Sanitizers are not supported for MSVC")
    endif()
else()
    message(STATUS "Sanitizers disabled")
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
