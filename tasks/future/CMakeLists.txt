cmake_minimum_required(VERSION 3.25)

project(future_task LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

add_executable(future_test test.cpp)
target_include_directories(future_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(future_test PRIVATE Threads::Threads GTest::gtest GTest::gtest_main)
