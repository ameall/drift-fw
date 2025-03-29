/**
 * @file buffer_manager.hpp
 * @brief 
 */

#include <cstdint>
#include <libcamera/libcamera.h>

#include <unordered_map>

class BufferManager {
  public:
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

    void unmap_all();

  private:
    std::unordered_map<libcamera::FrameBuffer*, uint8_t*> mappings;
};
