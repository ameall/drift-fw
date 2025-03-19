/**
 * @file camera.hpp
 * @brief Raspberry Pi GS Camera Manager for DRIFT Drone
 */

#pragma once

#include <libcamera/libcamera.h>

/**
 * @class Camera
 * @brief Abstracts the libcamera Camera class and manages interactions with
 *      the camera
 */
class Camera {
  public:
    /**
     * @brief Initializes class members
     */
    explicit Camera();

    /**
     * @brief Releases and resets camera and related objects
     */
    ~Camera();

    /**
     * @brief Configures and starts the Raspberry Pi GS camera
     *
     * @note if camera is unable to be found and acquired the application will
     *      exit
     *
     * @return 0
     */
    int8_t start_camera();

    /**
     * @brief Change the configured camera parameter values
     *
     * @param width pixel width of image to receive from sensor
     * @param height pixel height of image to receive from sensor
     * @return true if camera is able to be configured with given parameters
     * @return false if camera is unable to be configured with given parameters
     */
    bool change_config(const uint16_t width, const uint16_t height);

    /**
     * @brief Gets the actual camera device object
     *
     * @return libcamera Camera object representing the camera
     */
    std::shared_ptr<libcamera::Camera> get_camera() const;

    /**
     * @brief Gets the current configuration of the camera device object
     * 
     * @return libcamera CameraConfiguration object containing the camera
     *      configuration
     */
    std::shared_ptr<libcamera::CameraConfiguration> get_config() const;

    /**
     * @brief Prints all found camera devices
     */
    void print_cameras() const;

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
