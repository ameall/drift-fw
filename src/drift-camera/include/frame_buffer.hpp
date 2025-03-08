/**
 * @file frame_buffer.hpp
 * @brief Responsible for managing memory associated with sending requests for
 *      and receiving frames from the camera
 */

#pragma once

#include <libcamera/libcamera.h>
#include <memory>

#include "camera.hpp"
#include "model.hpp"

/**
 * @class FrameManager
 * @brief Abstracts the libcamera FrameManager class and manages memory and
 *      requests associated with receiving image frames from the camera
 */
class FrameManager {
  public:
    /**
     * @brief Initializes the camera class member with the given camera object
     *
     * @param camera object containing relevant camera device and configuration
     *      information
     */
    FrameManager(std::shared_ptr<Camera> camera);

    /**
     * @brief Releases the frame manager allocator used to manage memory
     */
    ~FrameManager();

    /**
     * @brief Allocates buffers for the camera
     *
     * @return 0 if buffers are successfully allocated; error value otherwise
     */
    int8_t setup_buffers();

    /**
     * @brief Creates a frame request for the camera
     *
     * @return 0 if the request is successfully created and added to the queue;
     *      error value otherwise
     */
    int8_t create_request();

    /**
     * @brief Starts the camera and sends frame requests
     *
     * @return 0 if the camera is successfully started; error value otherwise
     */
    int8_t get_frame();

    /**
     * @brief Adds requests to the queue
     */
    void queue_requests();

  private:
    /**
     * @brief Camera object containing the camera from which we will get images
     */
    std::shared_ptr<Camera> camera;

    /**
     * @brief Manages memory associated with frame buffers
     */
    std::unique_ptr<libcamera::FrameBufferAllocator> allocator;

    /**
     * @brief Manages image requests that will be sent to the camera
     */
    std::vector<std::unique_ptr<libcamera::Request>> requests;
};
