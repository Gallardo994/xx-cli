cmake_minimum_required(VERSION 3.22)

project(tests LANGUAGES CXX)
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests
    src/planner.cpp
    src/parser.cpp
    src/helpers.cpp
    src/renderers/inja_renderer.cpp
)
target_compile_features(tests PUBLIC cxx_std_23)
target_include_directories(tests PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/xx-lib/include
)
target_link_libraries(tests PRIVATE xx-lib gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
