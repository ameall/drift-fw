/**
 * @file camera.cpp
 * @brief Implementation of Raspberry Pi GS Camera Manager for DRIFT Drone
 */

#include <memory>
#include <string>

#include "camera.hpp"
#include "logging.hpp"

Camera::Camera() :
    camera_manager(std::make_unique<libcamera::CameraManager>()),
    camera(nullptr),
    camera_config(nullptr)
{}

Camera::~Camera()
{
    camera->release();
    camera_manager->stop();
    camera.reset();
    camera_config->end();
}

int8_t Camera::start_camera()
{
    camera_manager->start();

    print_cameras();

    auto cameras = camera_manager->cameras();

    if (cameras.empty()) {
        log_message(ERROR, "Camera::start_camera(): Camera manager did not detect any camera devices");
        camera_manager->stop();
        exit(EXIT_FAILURE);
    }

    std::string camera_id = cameras[0]->id();

    camera = camera_manager->get(camera_id);
    if (camera == nullptr) {
        log_message(ERROR, "Camera::start_camera(): Camera not found");
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

    libcamera::StreamConfiguration &stream_config = camera_config->at(0);
    log_message(INFO, "Camera::start_camera(): Default viewfinder configuration is: %s", stream_config.toString().c_str());

    camera->configure(camera_config.get());

    return 0;
}

bool Camera::change_config(const uint16_t width, const uint16_t height)
{
    // Width = 800, Height = 600 is default values. If we want to change these
    // values, we must verify the config before it gets applied to the camera
    libcamera::StreamConfiguration &stream_config = camera_config->at(0);
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

std::shared_ptr<libcamera::Camera> Camera::get_camera() const
{
    return camera;
}

std::shared_ptr<libcamera::CameraConfiguration> Camera::get_config() const
{
    return camera_config;
}

void Camera::print_cameras() const
{
    for (auto const &camera : camera_manager->cameras()) {
        log_message(INFO, "%s", camera->id().c_str());
    }
}
