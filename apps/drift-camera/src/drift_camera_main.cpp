/**
 * @file drift_camera_main.cpp
 * @brief Main running loop for drift-camera application
 */

#include "camera.hpp"
#include "frame_buffer.hpp"
#include "logging.hpp"

std::shared_ptr<Camera> camera = std::make_shared<Camera>();

/**
 * @brief Main program loop for drift-camera application
 */
int main()
{
    log_message(INFO, "main(): Starting drift-camera application");

    camera->start_camera();
    camera->print_cameras();

    FrameManager frame_manager(camera);
    frame_manager.setup_buffers();
    frame_manager.create_request();
    frame_manager.get_frames();

    while (1) {}

    return 0;
}
