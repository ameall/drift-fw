/**
 * @file drift_camera_main.cpp
 * @brief Main running loop for drift-camera application
 */

#include <libcamera/request.h>
#include <memory>
#include <unistd.h>

#include "camera.hpp"
#include "frame_buffer.hpp"

int main()
{
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();

    camera->start_camera();
    camera->print_cameras();

    FrameManager frame_manager(camera);
    frame_manager.setup_buffers();
    frame_manager.create_request();
    frame_manager.get_frame();
    frame_manager.queue_requests();
    frame_manager.queue_requests();
    frame_manager.queue_requests();
    frame_manager.queue_requests();

    sleep(3);

    return 0;
}
