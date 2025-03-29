/**
 * @file drift_camera_main.cpp
 * @brief Main running loop for drift-camera application
 */

#include "camera.hpp"
#include "frame_buffer.hpp"
#include "logging.hpp"
// #include "message.hpp"
 #include "model.hpp"
// #include "socket.hpp"

std::shared_ptr<Camera> camera = std::make_shared<Camera>();

/**
 * @brief Main program loop for drift-camera application
 */
int main()
{
    // Socket test
    // SocketManager socket_manager;
    // socket_manager.initialize_socket();
    // OutputsMessage message;
    // while (true) {
    //     message.create_message(1, 20000, 500, -500);
    //     socket_manager.send_message(message);
    //     message.clear_message();
    // }

    // return 0;

    // CV model test
    // Model model;
    // const std::string path_to_video = "/home/mealla/Documents/GitHub/drift-tracking/videos/IMG_4220.mp4";
    // const std::string path_to_video = "/home/drift/drift-tracking/videos/IMG_4220.mp4";
    // model.process_mp4(path_to_video);

    // return 0;

    log_message(INFO, "main(): Starting drift-camera application");

    camera->start_camera();
    camera->print_cameras();

    FrameManager frame_manager(camera);
    frame_manager.setup_buffers();
    frame_manager.create_request();
    frame_manager.get_frame();
    frame_manager.queue_requests();

    while (1) {
        // Need to add logic to allow flight control app to tell camera app to stop
    }

    return 0;
}
