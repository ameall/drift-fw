


#include "camera.hpp"

Camera::Camera() : camera_manager(std::make_unique<libcamera::CameraManager>()) {}

int8_t Camera::start_camera()
{
    camera_manager->start();

    return 0;
}

void Camera::print_cameras()
{
    for (auto const &camera : camera_manager->cameras()) {
        fprintf(stdout, "%s\n", camera->id());
    }
}
