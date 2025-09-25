cmake_minimum_required(VERSION 3.14)

project(
  shellScript
  VERSION 3.14
  DESCRIPTION "Shell Script Assignment 8"
  LANGUAGES C CXX)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

include(FetchContent)
FetchContent_Declare(
  googletest
  DOWNLOAD_EXTRACT_TIMESTAMP false
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Add all source files for the shell executable
file(GLOB SHELL_SOURCES src/shell.c src/internals.c src/history.c)

add_executable(shell ${SHELL_SOURCES})

add_executable(shell_test gtest/shell_test.cpp)
target_link_libraries(shell_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(shell_test)
