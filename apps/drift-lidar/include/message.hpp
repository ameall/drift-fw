#pragma once
/**
 * @file lidar_message.hpp
 * @brief Defines the message structure for LiDAR data communication.
 */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>

class LidarMessage {
public:
    explicit LidarMessage();
    ~LidarMessage() = default;

    void create_message(const int32_t message_id, const uint16_t distance);

    void clear_message();

    std::string get_message_as_string() const;

    bool is_message_valid() const;

private:
    bool message_valid;
    uint16_t distance;
};

#endif // LIDAR_MESSAGE_HPP
