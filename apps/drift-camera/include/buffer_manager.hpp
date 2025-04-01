/**
 * @file buffer_manager.hpp
 * @brief BufferManager class declaration
 */

#include <cstdint>
#include <libcamera/libcamera.h>

#include <unordered_map>

/**
 * @class BufferManager
 * @brief Abstracts management of the buffers used to store images captured
 *      from the camera
 */
class BufferManager {
  public:
    BufferManager() = default;

    ~BufferManager() = default;

    // Deleting copy and move constructors
    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;
    BufferManager(BufferManager&&) = delete;
    BufferManager& operator=(BufferManager&&) = delete;

    /**
     * @brief Maps buffers to file descriptors and stored them in a map so that
     *      they don't have to be remapped on each use
     *
     * @param buffer libcamera frame buffer to map
     */
    void map_buffer(libcamera::FrameBuffer *buffer);

    /**
     * @brief Gets a specific buffer from the buffer map
     *
     * @param buffer 
     * @return 
     */
    uint8_t* get_buffer(libcamera::FrameBuffer *buffer) const noexcept;

    /**
     * @brief Clears all buffer mappings
     */
    void unmap_all();

  private:
    std::unordered_map<libcamera::FrameBuffer*, uint8_t*> mappings;
};
