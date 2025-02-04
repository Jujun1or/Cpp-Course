cmake_minimum_required(VERSION 3.10)
project(SDLApp)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(app main.cpp)
target_link_libraries(app ${SDL2_LIBRARIES})