/**
 * @file message.hpp
 * @brief JSON message manager for sending distance readings from this lidar
 *      app to the flight control app
 */

#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;

class LidarMessage {
  public:
    /**
     * @brief Creates a JSON message object
     */
    explicit LidarMessage();
    ~LidarMessage() = default;

    /**
     * @brief Creates a JSON message to send over the socket
     *
     * @param message_id Unique identifier for each message
     * @param distance LiDAR distance reading
     */
    void create_message(const int32_t message_id, const uint16_t front_lidar_distance, const uint16_t down_lidar_distance);

    /**
     * @brief Clears the json message and resets the valid flag
     */
    void clear_message();

    /**
     * @brief Returns the message as a string instead of a JSON object
     *
     * @return string-ified JSON message
     */
    std::string get_message_as_string() const;

    /**
     * @brief Returns whether the JSON message is valid
     *
     * @return true if the message is valid; false otherwise
     */
    bool is_message_valid() const;

  private:
    bool message_valid;

    json message;
};
