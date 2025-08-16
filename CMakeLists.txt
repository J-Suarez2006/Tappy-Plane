cmake_minimum_required(VERSION 3.26)
project(Main C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_package(SDL3 REQUIRED)
add_subdirectory(SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

add_executable(Main)

target_sources(Main
PRIVATE
    Main.cpp
)

target_link_libraries(Main SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)