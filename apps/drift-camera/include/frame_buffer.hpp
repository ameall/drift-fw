/**
 * @file frame_buffer.hpp
 * @brief FrameManager class declaration
 */

#pragma once

#include <cstdint>
#include <libcamera/libcamera.h>
#include <memory>

#include "camera.hpp"

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
    explicit FrameManager(std::shared_ptr<Camera> camera);

    /**
     * @brief Releases the frame manager allocator used to manage memory
     */
    ~FrameManager();

    // Deleting copy and move constructors
    FrameManager(const FrameManager&) = delete;
    FrameManager& operator=(const FrameManager&) = delete;
    FrameManager(FrameManager&&) = delete;
    FrameManager& operator=(FrameManager&&) = delete;

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
     */
    void get_frames();

  private:
    std::shared_ptr<Camera> camera;
    std::unique_ptr<libcamera::FrameBufferAllocator> allocator;
    std::vector<std::unique_ptr<libcamera::Request>> requests;
};
