/**
 * @file frame_buffer.cpp
 * @brief Responsible for managing memory associated with sending request for
 *      and receiving frames from the camera
 */

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <jpeglib.h>
#include <libcamera/libcamera.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>
#include <sys/mman.h>
#include <thread>
#include <vector>

#include "camera.hpp"
#include "frame_buffer.hpp"
#include "frame_ops.hpp"
#include "logging.hpp"
#include "model.hpp"

extern std::shared_ptr<Camera> camera;

/**
 * @brief Callback function the libcamera Camera object will use to place image
 *      frames into the buffer
 *
 * @param request camera frame request that was sent to the camera
 */
static void request_complete(libcamera::Request *request)
{
    Model model;
    if (request->status() == libcamera::Request::RequestCancelled) {
        log_message(ERROR, "FrameManager::request_complete(): Frame request was cancelled");
        return;
    } else if (request->status() == libcamera::Request::RequestPending) {
        log_message(ERROR, "FrameManager::request_complete(): Frame request still pending");
        return;
    } else if (request->status() != libcamera::Request::RequestComplete) {
        log_message(ERROR, "FrameManager::request_complete(): Frame request incomplete");
        return;
    }

    const std::map<const libcamera::Stream *, libcamera::FrameBuffer *> &buffers = request->buffers();

    for (auto buffer_pair : buffers) {
        libcamera::FrameBuffer *buffer = buffer_pair.second;
        const libcamera::FrameMetadata &metadata = buffer->metadata();

        for (const libcamera::FrameMetadata::Plane &frame_plane : metadata.planes()) {
            log_message(INFO, "FrameManager::request_complete(): Metadata sequence %u; Bytes used: %u", metadata.sequence, frame_plane.bytesused);
        }
        libcamera::FrameBuffer::Plane plane = buffer->planes().at(0);

        // std::string filename = "image.jpg";
        // save_plane_to_jpeg(filename, plane);

        // cv::Mat image = cv::imread("image.jpg");
        // if (!image.empty()) {
        //     //cv::imshow("Loaded Image", image);
        //     model.process_frame(image);
        // }

        const uint8_t *xrgb_image_data = map_frame_buffer(plane);
        std::vector<uint8_t> rgb_data = convert_xrgb_to_rgb(xrgb_image_data, DEFAULT_CAMERA_WIDTH, DEFAULT_CAMERA_HEIGHT);
        cv::Mat image(DEFAULT_CAMERA_HEIGHT, DEFAULT_CAMERA_WIDTH, CV_8UC3, rgb_data.data());
        model.process_frame(image);
    }

    request->reuse(libcamera::Request::ReuseBuffers);
    camera->get_camera()->queueRequest(request);
}

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

void FrameManager::queue_requests()
{
    for (std::unique_ptr<libcamera::Request> &request : requests) {
        camera->get_camera()->queueRequest(request.get());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
