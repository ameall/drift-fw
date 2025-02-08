/**
 * @file camera.hpp
 * @brief Raspberry Pi GS Camera Manager for DRIFT Drone
 */

#pragma once

#include <libcamera/camera.h>
#include <libcamera/libcamera.h>
#include <libcamera/stream.h>
#include <memory>

class Camera {
  public:
    Camera();
    ~Camera();

    int8_t start_camera();

    bool change_config(uint16_t width, uint16_t height);

    std::shared_ptr<libcamera::Camera> get_camera();

    std::shared_ptr<libcamera::CameraConfiguration> get_config();

    void print_cameras();

  private:
    /**
     * @brief The CameraManager runs for the lifetime of the application and is
     *      responsible for abstracting the camera->application pipeline
     */
    std::unique_ptr<libcamera::CameraManager> camera_manager;

    /**
     * @brief The actual camera; must be acquired by the CameraManger
     */
    std::shared_ptr<libcamera::Camera> camera;

    /**
     * @brief Represents the current configuration of the camera
     */
    std::shared_ptr<libcamera::CameraConfiguration> camera_config;
};
