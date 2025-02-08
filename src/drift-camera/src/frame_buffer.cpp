/**
 * @file frame_buffer.cpp
 * @brief Responsible for sending requests for and receiving frames from the
 *      Camera
 */

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <jpeglib.h>
#include <libcamera/libcamera.h>
#include <memory>
#include <sys/mman.h>
#include <thread>
#include <vector>

#include "frame_buffer.hpp"
#include "logging.hpp"

// The camera object from main that allows us to requeue requests from the
// callback function
extern std::shared_ptr<Camera> camera;

// One byte each for R, G, and B
constexpr uint8_t NUM_COMPONENTS_RGB = 3u;
// One byte each for X, R, G, and B
constexpr uint8_t NUM_COMPONENTS_XRGB = 4u;
// Quality ranges from 0-100
constexpr uint8_t JPEG_QUALITY = 100u;

/**
 * @brief Allows for camera frames to be saved to disk as a JPEG image
 *
 * @param data buffer containing camera image data
 * @param width pixel width of the image
 * @param height pixel height of the image
 * @param filename Where to save the JPEG image to
 */
void save_jpeg_image(const uint8_t *data, int32_t width, int32_t height, const std::string &filename)
{
    struct jpeg_compress_struct compression_info;
    struct jpeg_error_mgr jpeg_error;
    compression_info.err = jpeg_std_error(&jpeg_error);
    jpeg_create_compress(&compression_info);

    std::FILE *outfile = fopen(filename.c_str(), "wb");
    if (!outfile) {
        log_message(ERROR, "Failed to open file for writing: %s", filename.c_str());
        return;
    }

    jpeg_stdio_dest(&compression_info, outfile);
    compression_info.image_width = width;
    compression_info.image_height = height;
    compression_info.input_components = NUM_COMPONENTS_RGB;
    compression_info.in_color_space = JCS_RGB;
    jpeg_set_defaults(&compression_info);
    jpeg_set_quality(&compression_info, JPEG_QUALITY, TRUE);
    jpeg_start_compress(&compression_info, TRUE);

    std::vector<uint8_t> rgb_data(width * height * NUM_COMPONENTS_RGB);
    for (int i = 0; i < width * height; i++) {
        rgb_data[i * NUM_COMPONENTS_RGB + 0] = data[i * NUM_COMPONENTS_XRGB + 2];  // R
        rgb_data[i * NUM_COMPONENTS_RGB + 1] = data[i * NUM_COMPONENTS_XRGB + 1];  // G
        rgb_data[i * NUM_COMPONENTS_RGB + 2] = data[i * NUM_COMPONENTS_XRGB + 0];  // B
    }

    while (compression_info.next_scanline < compression_info.image_height) {
        JSAMPROW row_pointer = (JSAMPROW)&rgb_data[compression_info.next_scanline * width * NUM_COMPONENTS_RGB];
        jpeg_write_scanlines(&compression_info, &row_pointer, 1);
    }

    jpeg_finish_compress(&compression_info);
    jpeg_destroy_compress(&compression_info);
    fclose(outfile);
}

/**
 * @brief Remaps the image frame plane into a data buffer
 *
 * @param plane image plane to remap
 */
static void *map_frame_buffer(const libcamera::FrameBuffer::Plane &plane)
{
    void *mapped_memory = mmap(nullptr, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), plane.offset);
    if (mapped_memory == MAP_FAILED) {
        log_message(ERROR, "Failed to map memory");
        return nullptr;
    }
    return mapped_memory;
}

/**
 * @brief Callback function the libcamera Camera object will use to place image
 *      frames into the buffer
 *
 * @param request camera frame request that was sent to the camera
 */
static void request_complete(libcamera::Request *request)
{
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

        void *data = map_frame_buffer(plane);

        const uint8_t *pixel_data = static_cast<const uint8_t *>(data);
        int width = 800;
        int height = 600;

        std::string filename = "image1.jpg";
        save_jpeg_image(pixel_data, width, height, filename);
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
