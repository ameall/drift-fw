/**
 * @file frame_ops.cpp
 * @brief Defines common operations that can be performed after a frame has
 *      been received
 */

#include <cstdio>
#include <jpeglib.h>
#include <memory>
#include <sys/mman.h>
#include <vector>

#include "camera.hpp"
#include "frame_ops.hpp"
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
 * @brief Remaps the image frame plane into a data buffer
 *
 * @param plane image plane to remap
 * @return buffer containing re-mapped image data
 */
 uint8_t *map_frame_buffer(const libcamera::FrameBuffer::Plane &plane)
{
    void *mapped_memory = mmap(nullptr, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), plane.offset);
    if (mapped_memory == MAP_FAILED) {
        log_message(ERROR, "Failed to map memory");
        return nullptr;
    }

    return static_cast<uint8_t *>(mapped_memory);
}

/**
 * @brief Converts an array of XRGB data into an array of RGB data
 *
 * @param xrgb_data Array of XRGB data to convert
 * @param width Pixel width of image represented by XRGB data
 * @param height Pixel height of image represented by XRGB data
 * @return Vector containing RGB image data
 */
std::vector<uint8_t> convert_xrgb_to_rgb(const uint8_t* xrgb_data, const uint32_t width, const uint32_t height)
{
    // XRGB -> RGB data conversion to prep image for writing to JPEG
    std::vector<uint8_t> rgb_data(width * height * NUM_COMPONENTS_RGB);
    for (int i = 0; i < width * height; i++) {
        rgb_data[i * NUM_COMPONENTS_RGB + 0] = xrgb_data[i * NUM_COMPONENTS_XRGB + 2];  // R
        rgb_data[i * NUM_COMPONENTS_RGB + 1] = xrgb_data[i * NUM_COMPONENTS_XRGB + 1];  // G
        rgb_data[i * NUM_COMPONENTS_RGB + 2] = xrgb_data[i * NUM_COMPONENTS_XRGB + 0];  // B
    }
    return rgb_data;
}

cv::Mat convert_plane_to_mat(const libcamera::FrameBuffer::Plane &plane)
{
    const uint8_t* xrgb_image_data = map_frame_buffer(plane);
    std::vector<uint8_t> rgb_data = convert_xrgb_to_rgb(xrgb_image_data, DEFAULT_CAMERA_WIDTH, DEFAULT_CAMERA_HEIGHT);
    cv::Mat image(DEFAULT_CAMERA_HEIGHT, DEFAULT_CAMERA_WIDTH, CV_8UC3, rgb_data.data());
    return image;
}

bool save_plane_to_jpeg(const std::string filename, const libcamera::FrameBuffer::Plane &plane, const uint32_t width, const uint32_t height)
{
    std::FILE *jpeg_file = fopen(filename.c_str(), "wb");
    if (!jpeg_file) {
        log_message(ERROR, "save_plane_to_jpeg(): Failed to open destination JPEG file: %s", filename.c_str());
        return false;
    }

    const uint8_t *xrgb_image_data = map_frame_buffer(plane);
    if (xrgb_image_data == nullptr) {
        log_message(ERROR, "save_plane_to_jpeg(): Failed to remap image plane to data buffer");
        return false;
    }

    struct jpeg_compress_struct compressed_image;
    struct jpeg_error_mgr jpeg_error;

    compressed_image.err = jpeg_std_error(&jpeg_error);
    jpeg_create_compress(&compressed_image);
    jpeg_stdio_dest(&compressed_image, jpeg_file);

    compressed_image.image_width = width;
    compressed_image.image_height = height;
    compressed_image.input_components = NUM_COMPONENTS_RGB;
    compressed_image.in_color_space = JCS_RGB;

    jpeg_set_defaults(&compressed_image);
    jpeg_set_quality(&compressed_image, JPEG_QUALITY, TRUE);
    jpeg_start_compress(&compressed_image, TRUE);

    std::vector<uint8_t> rgb_data = convert_xrgb_to_rgb(xrgb_image_data, width, height);

    // Passes each row of RGB for compression, one at a time
    while (compressed_image.next_scanline < compressed_image.image_height) {
        JSAMPROW row_pointer = (JSAMPROW)&rgb_data[compressed_image.next_scanline * width * NUM_COMPONENTS_RGB];
        jpeg_write_scanlines(&compressed_image, &row_pointer, 1);
    }

    jpeg_finish_compress(&compressed_image);
    jpeg_destroy_compress(&compressed_image);
    fclose(jpeg_file);

    return true;
}
