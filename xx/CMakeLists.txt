cmake_minimum_required(VERSION 3.22)

project(xx LANGUAGES CXX)

set(SOURCES
    src/main.cpp
)

add_executable(xx ${SOURCES})
target_compile_features(xx PUBLIC cxx_std_23)
target_include_directories(xx PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/xx-lib/include
)
target_link_libraries(xx PRIVATE xx-lib spdlog::spdlog)
