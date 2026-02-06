cmake_minimum_required(VERSION 3.22)

project(xx-lib LANGUAGES CXX)
include(FetchContent)

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(inja CONFIG REQUIRED)
find_package(Lua REQUIRED)

set(SOURCES
    src/detail/command.cpp
    src/detail/parser.cpp
    src/detail/planner.cpp
    src/detail/platform.cpp

    src/detail/executor.cpp
    src/detail/executors/lua_executor.cpp
    src/detail/executors/dotnet_run_executor.cpp

    src/detail/renderer.cpp
    src/detail/renderers/inja_renderer.cpp

    src/detail/luavm.cpp
    src/detail/luavm_modules/luavm_json.cpp

    src/detail/helpers.cpp
)

if (WIN32)
    message(STATUS "Configuring for Windows platform")
    list(APPEND SOURCES src/detail/executors/platform_executor_windows.cpp)
elseif(UNIX)
    message(STATUS "Configuring for UNIX platform")
    list(APPEND SOURCES src/detail/executors/platform_executor_unix.cpp)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

add_library(xx-lib ${SOURCES})
target_compile_features(xx-lib PUBLIC cxx_std_23)
target_include_directories(xx-lib PRIVATE include ${LUA_INCLUDE_DIR})
target_link_libraries(xx-lib PRIVATE
    spdlog::spdlog_header_only
    yaml-cpp::yaml-cpp
    nlohmann_json::nlohmann_json
    pantor::inja
    ${LUA_LIBRARIES}
)
