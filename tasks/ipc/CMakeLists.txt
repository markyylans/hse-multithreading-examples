cmake_minimum_required(VERSION 3.25)

project(ipc_tasks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(producer producer.cpp)
target_link_libraries(producer PRIVATE Threads::Threads)

add_executable(consumer consumer.cpp)
target_link_libraries(consumer PRIVATE Threads::Threads)
