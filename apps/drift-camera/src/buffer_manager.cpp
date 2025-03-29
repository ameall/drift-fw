/**
 * @file buffer_manager.cpp
 * @brief 
 */

#include <cstdio>
#include <sys/mman.h>

#include "buffer_manager.hpp"

void BufferManager::map_buffer(libcamera::FrameBuffer *buffer)
{
    libcamera::FrameBuffer::Plane plane = buffer->planes().at(0);

    void *ptr = mmap(nullptr, plane.length, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), plane.offset);

    mappings[buffer] = static_cast<uint8_t*>(ptr);
}

uint8_t* BufferManager::get_buffer(libcamera::FrameBuffer *buffer) const noexcept
{
    return mappings.at(buffer);
}

void BufferManager::unmap_all()
{
    for (auto &[buffer, ptr] : mappings) {
         munmap(ptr, buffer->planes().at(0).length);
    }
    mappings.clear();
}
