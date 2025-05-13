cmake_minimum_required(VERSION 3.10)
project(SDLApp)

set(CMAKE_CXX_STANDARD 17)

# Поиск SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Альтернативный способ поиска SDL2_ttf
find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf)
find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h PATH_SUFFIXES SDL2)

if(SDL2_TTF_LIBRARY AND SDL2_TTF_INCLUDE_DIR)
    message(STATUS "Found SDL2_ttf: ${SDL2_TTF_LIBRARY}")
    set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
    set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})
else()
    message(FATAL_ERROR "SDL2_ttf not found! Please install libsdl2-ttf-dev")
endif()

add_executable(app 
    main.cpp
    space_invaders.cpp
)

target_include_directories(app PRIVATE 
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

target_link_libraries(app 
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)

# Копируем ресурсы в директорию сборки
configure_file(arialmt.ttf ${CMAKE_CURRENT_BINARY_DIR}/arialmt.ttf COPYONLY)