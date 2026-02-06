cmake_minimum_required(VERSION 3.22)

project(xx LANGUAGES CXX)

find_package(CLI11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(semver CONFIG REQUIRED)

set(SOURCES
    src/main.cpp
)

add_executable(xx ${SOURCES})
target_compile_features(xx PUBLIC cxx_std_23)
target_include_directories(xx PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/xx-lib/include
)
target_link_libraries(xx PRIVATE
    xx-lib
    CLI11::CLI11
    spdlog::spdlog_header_only
    cpr::cpr
    nlohmann_json::nlohmann_json
    semver::semver
)
