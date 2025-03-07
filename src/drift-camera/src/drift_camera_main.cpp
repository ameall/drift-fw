/**
 * @file drift_camera_main.cpp
 * @brief Main running loop for drift-camera application
 */

#include <chrono>
#include <libcamera/libcamera.h>
#include <thread>

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
    frame_manager.get_frame();
    frame_manager.queue_requests();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    return 0;
}
