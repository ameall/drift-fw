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
#include "model.hpp"

std::shared_ptr<Camera> camera = std::make_shared<Camera>();

/**
 * @brief Main program loop for drift-camera application
 */
int main()
{
    //Model model;

    //const std::string path_to_video = "/home/drift/drift-tracking/videos/IMG_4220.mp4";
    //model.process_mp4(path_to_video);

    log_message(INFO, "main(): Starting drift-camera application");
    camera->start_camera();
    camera->print_cameras();

    FrameManager frame_manager(camera);
    frame_manager.setup_buffers();
    frame_manager.create_request();
    frame_manager.get_frame();
    frame_manager.queue_requests();

    while (1) {}

    return 0;
}
