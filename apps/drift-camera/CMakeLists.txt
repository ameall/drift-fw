# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Create the project
project(drift-camera
    VERSION 1.0
    DESCRIPTION "DRIFT Raspberry Pi Global Shutter Camera Image Processing Application"
    LANGUAGES CXX
)

# Set C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Provides compile_commands.json for LSP autocompletion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

add_compile_options(
    -lcamera
    -ljpeg
)

# Use PkgConfig to link libcamera
find_package(PkgConfig REQUIRED)
find_package(JPEG REQUIRED)
find_package(OpenCV REQUIRED)

# Pull Nlohmann's JSON library from GitHub
include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3  # Use the latest stable version
)

FetchContent_MakeAvailable(json)

# Import libcamera libraries
pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)
message(STATUS "libcamera library found:")
message(STATUS "    Version: ${LIBCAMERA_VERSION}")
message(STATUS "    Libraries: ${LIBCAMERA_LINK_LIBRARIES}")
message(STATUS "    Include Path: ${LIBCAMERA_INCLUDE_DIRS}")

# Add source files
add_executable(drift-camera
    ./src/drift_camera_main.cpp
    ./src/camera.cpp
    ./src/message.cpp
    ./src/frame_buffer.cpp
    ./src/logging.cpp
    ./src/model.cpp
    ./src/socket.cpp
    ./src/buffer_manager.cpp
)

# Necesssary includes
target_include_directories(drift-camera PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    ${CMAKE_SOURCE_DIR}
    ${LIBCAMERA_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(drift-camera PkgConfig::LIBCAMERA)
target_link_libraries(drift-camera ${JPEG_LIBRARIES})
target_link_libraries(drift-camera ${OpenCV_LIBS})
target_link_libraries(drift-camera nlohmann_json::nlohmann_json)

# Add source files
add_executable(test-script
    ../../scripts/camera_test.cpp
)

# Necesssary includes
target_include_directories(test-script PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    ${CMAKE_SOURCE_DIR}
    ${LIBCAMERA_INCLUDE_DIRS}
)

target_link_libraries(test-script PkgConfig::LIBCAMERA)
target_link_libraries(test-script ${JPEG_LIBRARIES})
