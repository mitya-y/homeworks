cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)

project(z-levels)
add_executable(${CMAKE_PROJECT_NAME} main.cpp window.cpp drawing.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} GL GLU glut)