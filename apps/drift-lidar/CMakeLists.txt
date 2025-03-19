# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Create the project
project(lidar-app
    VERSION 1.0
    DESCRIPTION ""
    LANGUAGES CXX
)

# Set C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Provides compile_commands.json for LSP autocompletion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Add source files
add_executable(lidar-app
    ./src/lidar.cpp
    ./src/lidar_app_main.cpp
)

# Necesssary includes
target_include_directories(lidar-app PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    ${CMAKE_SOURCE_DIR}
)
