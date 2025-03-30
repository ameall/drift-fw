/**
 * @file frame_buffer.cpp
 * @brief Responsible for managing memory associated with sending request for
 *      and receiving frames from the camera
 */

#include <cstdio>
#include <jpeglib.h>
#include <opencv2/opencv.hpp>
#include <sys/mman.h>
#include <vector>

#include "buffer_manager.hpp"
#include "frame_buffer.hpp"
#include "logging.hpp"
#include "model.hpp"

extern std::shared_ptr<Camera> camera;

static BufferManager buffer_manager;

/**
 * @brief Callback function the libcamera Camera object will use to place image
 *      frames into the buffer
 *
 * @param request camera frame request that was sent to the camera
 */
static void request_complete(libcamera::Request *request)
{
    Model model;

    if (request->status() != libcamera::Request::RequestComplete) {
        log_message(ERROR, "request_complete(): Frame request was not completed");
    }

    for (auto &[stream, buffer] : request->buffers()) {
        const libcamera::FrameMetadata &metadata = buffer->metadata();

        for (const libcamera::FrameMetadata::Plane &frame_plane : metadata.planes()) {
            log_message(INFO, "FrameManager::request_complete(): Metadata sequence %u; Bytes used: %u", metadata.sequence, frame_plane.bytesused);
        }

        uint8_t* xrgb_data = buffer_manager.get_buffer(buffer);
        cv::Mat xrgb_image(camera->get_config()->at(0).size.height, camera->get_config()->at(0).size.width, CV_8UC4, xrgb_data);
        cv::Mat rgb_image;
        cv::cvtColor(xrgb_image, rgb_image, cv::COLOR_BGRA2BGR);
        model.process_frame(rgb_image);
    }

    request->reuse(libcamera::Request::ReuseBuffers);
    camera->get_camera()->queueRequest(request);
}

FrameManager::FrameManager(std::shared_ptr<Camera> camera) : camera(camera) {}

FrameManager::~FrameManager()
{
    allocator->free(camera->get_config()->at(0).stream());
}

int8_t FrameManager::setup_buffers()
{
    allocator = std::unique_ptr<libcamera::FrameBufferAllocator>(new libcamera::FrameBufferAllocator(camera->get_camera()));
    for (libcamera::StreamConfiguration &stream_config : *camera->get_config()) {
        stream_config.bufferCount = 4;
        int8_t ret = allocator->allocate(stream_config.stream());
        if (ret < 0) {
            log_message(ERROR, "FrameManager::setup_buffers(): Buffer allocation failed");
            return -ENOMEM;
        }

        size_t allocated = allocator->buffers(stream_config.stream()).size();
        log_message(INFO, "FrameManager::setup_buffers(): Allocated %zu buffers for stream", allocated);
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
            log_message(ERROR, "FrameManager::create_request(): Failed to create frame request");
            return -ENOMEM;
        }

        const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[1];
        int8_t ret = request->addBuffer(stream, buffer.get());
        if (ret < 0) {
            log_message(ERROR, "FrameManager::create_request(): Can't create buffer for request");
            return ret;
        }

        buffer_manager.map_buffer(&(*buffer));

        requests.push_back(std::move(request));
    }

    return 0;
}

void FrameManager::get_frames()
{
    camera->get_camera()->requestCompleted.connect(request_complete);
    camera->get_camera()->start();

    for (std::unique_ptr<libcamera::Request> &request : requests) {
        camera->get_camera()->queueRequest(request.get());
    }
}
