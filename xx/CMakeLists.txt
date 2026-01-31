cmake_minimum_required(VERSION 3.22)

project(xx-cli VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    src/detail/parser.cpp
    src/detail/planner.cpp
    src/detail/executor.cpp
    src/detail/platform.cpp
    src/main.cpp
)

add_executable(xx-cli ${SOURCES})
target_compile_features(xx-cli PRIVATE cxx_std_23)
target_include_directories(xx-cli PRIVATE include)
