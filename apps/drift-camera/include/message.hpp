/**
 * @file message.hpp
 * @brief JSON message manager for sending camera image processing outputs from
 *      this camera app to the flight control app
 */

#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;

class OutputsMessage {
  public:
    /**
     * @brief Creates a JSON message object
     */
    explicit OutputsMessage();
    ~OutputsMessage() = default;

    /**
     * @brief Creates a JSON message to send over the socket
     *
     * @param message_id Unique identifier for each message
     * @param detection_area Area of the detection box
     * @param delta_x X offset between the center of the detection box and the
     *      center of the image (detection_center - image_center)
     * @param delta_y Y yffset between the center of the detection box and the
     *      center of the image (detection_center - image_center)
     */
    void create_message(const int32_t message_id, const int32_t detection_area, const int16_t delta_x, const int16_t delta_y);

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
