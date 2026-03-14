cmake_minimum_required(VERSION 3.25)

project(futex_tasks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(futex_test test.cpp)
target_link_libraries(futex_test PRIVATE Threads::Threads GTest::gtest GTest::gtest_main)
