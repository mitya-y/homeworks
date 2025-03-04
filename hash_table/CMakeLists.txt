cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 23)
project(hashtable)
add_executable(${CMAKE_PROJECT_NAME} main.cpp hash_table.cpp tests.cpp)