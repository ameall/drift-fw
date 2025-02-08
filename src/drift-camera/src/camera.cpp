/**
 * @file camera.cpp
 * @brief Implementation of Raspberry Pi GS Camera Manager for DRIFT Drone
 *
 * @copyright DRIFT 2025
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <libcamera/camera.h>
#include <libcamera/stream.h>
#include <memory>

#include "camera.hpp"

Camera::Camera() :
    camera_manager(std::make_unique<libcamera::CameraManager>()),
    camera(nullptr),
    camera_config(nullptr)
{}

Camera::~Camera()
{
    camera_manager->stop();
    camera->stop();
    camera->release();
    camera.reset();
    camera_config->end();
}

int8_t Camera::start_camera()
{
    camera_manager->start();

    print_cameras();

    auto cameras = camera_manager->cameras();

    if (cameras.empty()) {
        fprintf(stderr, "Camera::start_camera(): Camera manager did not detect any camera devices\n");
        camera_manager->stop();
        exit(EXIT_FAILURE);
    }

    std::string camera_id = cameras[0]->id();

    camera = camera_manager->get(camera_id);
    if (camera == nullptr) {
        fprintf(stderr, "Camera::start_camera(): Camera not found\n");
        exit(EXIT_FAILURE);
    }

    camera->acquire();
    if (camera == nullptr) {
        fprintf(stderr, "Camera::start_camera(): Failed to acquire camera\n");
        exit(EXIT_FAILURE);
    }

    camera_config = camera->generateConfiguration( { libcamera::StreamRole::Viewfinder } );
    if (camera_config == nullptr) {
        fprintf(stderr, "Camera::start_camera(): Failed to generate camera configuration\n");
        exit(EXIT_FAILURE);
    }

    libcamera::StreamConfiguration &stream_config = camera_config->at(0);
    fprintf(stdout, "Camera::start_camera(): Default viewfinder configuration is: %s\n", stream_config.toString().c_str());

    camera->configure(camera_config.get());

    return 0;
}

bool Camera::change_config(uint16_t width, uint16_t height)
{
    // Width = 800, Height = 600 is default values. If we want to change these
    // values, we must verify the config before it gets applied to the camera
    libcamera::StreamConfiguration &stream_config = camera_config->at(0);
    stream_config.size.width = width;
    stream_config.size.height = height;

    switch (camera_config->validate()) {
        case libcamera::CameraConfiguration::Valid:
            fprintf(stdout, "Camera::change_config(): New camera parameters successfully validated\n");
            break;
        case libcamera::CameraConfiguration::Adjusted:
            fprintf(stdout, "Camera::change_config(): New camera parameters adjusted successfully\n");
            break;
        case libcamera::CameraConfiguration::Invalid:
            fprintf(stdout, "Camera::change_config(): New camera parameters were not able to be applied\n");
            return false;
            break;
        default:
            break;
    }

    camera->configure(camera_config.get());

    return true;
}

std::shared_ptr<libcamera::Camera> Camera::get_camera()
{
    return camera;
}

std::shared_ptr<libcamera::CameraConfiguration> Camera::get_config()
{
    return camera_config;
}

void Camera::print_cameras()
{
    for (auto const &camera : camera_manager->cameras()) {
        fprintf(stdout, "%s\n", camera->id().c_str());
    }
}
