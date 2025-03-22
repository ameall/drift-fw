# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Create the project
project(drift-lidar
    VERSION 1.0
    DESCRIPTION "DRIFT LiDAR Control Application"
    LANGUAGES CXX
)

# Set C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Provides compile_commands.json for LSP autocompletion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Pull Nlohmann's JSON library from GitHub
include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3  # Use the latest stable version
)

FetchContent_MakeAvailable(json)

# Add source files
add_executable(drift-lidar
    ./src/lidar.cpp
    ./src/drift_lidar_main.cpp
    ./src/logging.cpp
    ./src/message.cpp
    ./src/socket.cpp
)

# Necesssary includes
target_include_directories(drift-lidar PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(drift-lidar nlohmann_json::nlohmann_json)
