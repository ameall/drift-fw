/**
 * @file frame_buffer.hpp
 * @brief 
 */

#pragma once

#include "camera.hpp"
#include <cwchar>
#include <libcamera/camera.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <memory>

class FrameManager {
  public:
    FrameManager(std::shared_ptr<Camera> camera);
    ~FrameManager();

    int8_t setup_buffers();

    int8_t create_request();

    int8_t get_frame();

    void queue_requests();

    void requeue_requests();

    static void request_complete(libcamera::Request *request);

  private:
    std::shared_ptr<Camera> camera;

    std::unique_ptr<libcamera::FrameBufferAllocator> allocator;

    std::vector<std::unique_ptr<libcamera::Request>> requests;
};
