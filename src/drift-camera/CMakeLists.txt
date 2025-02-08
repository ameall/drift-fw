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

add_compile_options(
    -lcamera
)

# Use PkgConfig to link libcamera
find_package(PkgConfig REQUIRED)

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
    ./src/frame_buffer.cpp
)

# Necesssary includes
target_include_directories(drift-camera PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    ${CMAKE_SOURCE_DIR}
    ${LIBCAMERA_INCLUDE_DIRS}
)

target_link_libraries(drift-camera PkgConfig::LIBCAMERA)

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
