/**
 * @file frame_ops.hpp
 * @brief Defines common operations that can be performed after a frame has
 *      been received
 */

#include <cstdint>
#include <libcamera/libcamera.h>
#include <opencv2/opencv.hpp>
#include <string>

// Default camera parameters
constexpr uint16_t DEFAULT_CAMERA_WIDTH = 800u;
constexpr uint16_t DEFAULT_CAMERA_HEIGHT = 600u;

/**
 * @brief Remaps the image frame plane into a data buffer
 *
 * @param plane image plane to remap
 * @return buffer containing re-mapped image data
 */
 uint8_t *map_frame_buffer(const libcamera::FrameBuffer::Plane &plane);

/**
 * @brief Converts an array of XRGB data into an array of RGB data
 *
 * @param xrgb_data Array of XRGB data to convert
 * @param width Pixel width of image represented by XRGB data
 * @param height Pixel height of image represented by XRGB data
 * @return Vector containing RGB image data
 */
std::vector<uint8_t> convert_xrgb_to_rgb(const uint8_t* xrgb_data, const uint32_t width, const uint32_t height);

/**
 * @brief Converts an RGB array into a cv::Mat 2-D array
 *
 * @param plane single plane of image data from a frame
 * @return 2-D cv::Mat object containing image data
 */
cv::Mat convert_plane_to_mat(const libcamera::FrameBuffer::Plane &plane);

/**
 * @brief Allows for camera frames to be saved to disk as a JPEG image
 *
 * @param filename name of file to save JPEG image to (don't include extension)
 * @param plane single plane of image data from a frame
 * @param width pixel width of the frame
 * @param height pixel height of the frame
 * @return true if the frame is successfully saved to disk
 * @return false if the frame is not successfully saved to disk
 */
bool save_plane_to_jpeg(const std::string filename, const libcamera::FrameBuffer::Plane &plane, const uint32_t width = DEFAULT_CAMERA_WIDTH, const uint32_t height = DEFAULT_CAMERA_HEIGHT);
