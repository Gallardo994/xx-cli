cmake_minimum_required(VERSION 3.22)

project(xx-lib LANGUAGES CXX)
include(FetchContent)

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
    src/third_party/nlohmann_lua.cpp

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

FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/walterschell/Lua.git
    GIT_TAG v5.4.7
)
FetchContent_MakeAvailable(lua)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG f0329d0efd2e62ceda5ee899c343ee445f6f7bd2
)
FetchContent_MakeAvailable(yaml-cpp)

add_library(xx-lib ${SOURCES})
target_compile_features(xx-lib PUBLIC cxx_std_23)
target_include_directories(xx-lib PRIVATE include)
target_link_libraries(xx-lib PRIVATE spdlog::spdlog lua_static yaml-cpp)
