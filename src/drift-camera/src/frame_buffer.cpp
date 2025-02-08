/**
 * @file frame_buffer.cpp
 * @brief Responsible for sending requests for and receiving frames from the
 *      Camera
 *
 * @copyright DRIFT 2025
 */

#include "frame_buffer.hpp"
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <libcamera/camera.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>
#include <memory>
#include <thread>
#include <unistd.h>
#include <vector>

FrameManager::FrameManager(std::shared_ptr<Camera> camera) : camera(camera)
{
}

FrameManager::~FrameManager()
{
    allocator->free(camera->get_config()->at(0).stream());
}

int8_t FrameManager::setup_buffers()
{
    allocator = std::unique_ptr<libcamera::FrameBufferAllocator>(new libcamera::FrameBufferAllocator(camera->get_camera()));
    for (libcamera::StreamConfiguration &stream_config : *camera->get_config()) {
        int8_t ret = allocator->allocate(stream_config.stream());
        if (ret < 0) {
            fprintf(stderr, "FrameManager::setup_buffers(): Buffer allocation failed\n");
            return -ENOMEM;
        }

        size_t allocated = allocator->buffers(stream_config.stream()).size();
        fprintf(stdout, "FrameManager::setup_buffers(): Allocated %zu buffers for stream\n", allocated);
    }

    return 0;
}

int8_t FrameManager::create_request()
{
    libcamera::Stream *stream = camera->get_config()->at(0).stream();
    const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = allocator->buffers(stream);

    for (uint32_t i = 0; i < buffers.size(); ++i) {
        std::unique_ptr<libcamera::Request> request = camera->get_camera()->createRequest();
        if (!request) {
            fprintf(stderr, "FrameManager::create_request(): Failed to create frame request\n");
            return -ENOMEM;
        }

        const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[1];
        int8_t ret = request->addBuffer(stream, buffer.get());
        if (ret < 0) {
            fprintf(stderr, "FrameManager::create_request(): Can't create buffer for request\n");
            return ret;
        }

        requests.push_back(std::move(request));
    }

    return 0;
}

int8_t FrameManager::get_frame()
{
    camera->get_camera()->requestCompleted.connect(request_complete);
    camera->get_camera()->start();
    return 0;
}

void FrameManager::request_complete(libcamera::Request *request)
{
    if (request->status() == libcamera::Request::RequestCancelled) {
        fprintf(stderr, "FrameManager::request_complete(): Frame request was cancelled\n");
        return;
    } else if (request->status() == libcamera::Request::RequestPending) {
        fprintf(stderr, "FrameManager::request_complete(): Frame request still pending\n");
        return;
    } else if (request->status() != libcamera::Request::RequestComplete) {
        fprintf(stderr, "FrameManager::request_complete(): Frame request incomplete\n");
        return;
    }

    const std::map<const libcamera::Stream *, libcamera::FrameBuffer *> &buffers = request->buffers();

    for (auto buffer_pair : buffers) {
        libcamera::FrameBuffer *buffer = buffer_pair.second;
        const libcamera::FrameMetadata &metadata = buffer->metadata();

        fprintf(stdout, "FrameManager::request_complete(): Metadata sequence %u; Bytes used: ", metadata.sequence);

        uint8_t num_planes = 0;
        for (const libcamera::FrameMetadata::Plane &frame_plane : metadata.planes()) {
            fprintf(stdout, "%u", frame_plane.bytesused);

            if (++num_planes < metadata.planes().size()) {
                fprintf(stdout, " / ");
            }

            fprintf(stdout, "\n");
            fflush(stdout);
            // The actual image data within a buffer may be written to disk using
            // the FileSink class
        }
    }
}

void FrameManager::requeue_requests()
{
    //this->camera->queueRequest(request);
}

void FrameManager::queue_requests()
{
    for (std::unique_ptr<libcamera::Request> &request : requests) {
        camera->get_camera()->queueRequest(request.get());
    }

    sleep(3);
}
