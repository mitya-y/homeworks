cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 23)
project(det)
add_executable(${CMAKE_PROJECT_NAME} main.cpp tests.cpp det.cpp)