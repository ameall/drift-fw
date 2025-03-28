/**
 * @file camera.cpp
 * @brief Implementation of Raspberry Pi GS Camera Manager for DRIFT Drone
 */

#include <memory>
#include <string>
#include <sys/mman.h>

#include "camera.hpp"
#include "logging.hpp"

// Default camera parameters
constexpr uint16_t DEFAULT_CAMERA_WIDTH = 800u;
constexpr uint16_t DEFAULT_CAMERA_HEIGHT = 600u;

Camera::Camera() :
    camera_manager(std::make_unique<libcamera::CameraManager>()),
    camera(nullptr),
    camera_config(nullptr)
{}

Camera::~Camera()
{
    if (camera) {
        camera->release();
    }
    if (camera_manager) {
        camera_manager->stop();
    }
    camera.reset();
    if (camera_config) {
        camera_config->end();
    }
}

int8_t Camera::start_camera()
{
    camera_manager->start();
    // print_cameras();

    auto cameras = camera_manager->cameras();
    if (cameras.empty()) {
        log_message(ERROR, "Camera::start_camera(): Camera manager did not detect any camera devices");
        camera_manager->stop();
        exit(EXIT_FAILURE);
    }

    camera = camera_manager->get(cameras[0]->id());
    if (camera == nullptr) {
        log_message(ERROR, "Camera::start_camera(): Failed to acquire camera");
        exit(EXIT_FAILURE);
    }

    camera->acquire();
    if (camera == nullptr) {
        log_message(ERROR, "Camera::start_camera(): Failed to acquire camera");
        exit(EXIT_FAILURE);
    }

    camera_config = camera->generateConfiguration( { libcamera::StreamRole::Viewfinder } );
    if (camera_config == nullptr) {
        log_message(ERROR, "Camera::start_camera(): Failed to generate camera configuration");
        exit(EXIT_FAILURE);
    }

    // libcamera::StreamConfiguration &stream_config = camera_config->at(0);
    // log_message(INFO, "Camera::start_camera(): Default viewfinder configuration is: %s", stream_config.toString().c_str());

    camera->configure(camera_config.get());

    return 0;
}

bool Camera::change_config(const uint16_t width, const uint16_t height)
{
    if (!camera_config) {
        log_message(ERROR, "Camera::change_config(): Camera configuration is not initialized");
        return false;
    }

    libcamera::StreamConfiguration &stream_config = camera_config->at(0);
    if (width == stream_config.size.width && height == stream_config.size.height) {
        return true;
    }

    stream_config.size.width = width;
    stream_config.size.height = height;

    switch (camera_config->validate()) {
        case libcamera::CameraConfiguration::Valid:
            log_message(INFO, "Camera::change_config(): New camera parameters successfully validated");
            break;
        case libcamera::CameraConfiguration::Adjusted:
            log_message(INFO, "Camera::change_config(): New camera parameters adjusted successfully");
            break;
        case libcamera::CameraConfiguration::Invalid:
            log_message(INFO, "Camera::change_config(): New camera parameters were not able to be applied");
            return false;
            break;
        default:
            break;
    }

    camera->configure(camera_config.get());

    return true;
}

std::shared_ptr<libcamera::Camera> Camera::get_camera() const noexcept
{
    return camera;
}

std::shared_ptr<libcamera::CameraConfiguration> Camera::get_config() const noexcept
{
    return camera_config;
}

void Camera::print_cameras() const noexcept
{
    if (!camera_manager) {
        return;
    }

    for (auto const &camera : camera_manager->cameras()) {
        log_message(INFO, "%s", camera->id().c_str());
    }
}
