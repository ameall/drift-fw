/**
 * @file message.hpp
 * @brief JSON message manager for sending camera image processing outputs from
 *      this camera app to the flight control app
 */

#pragma once

#include <nlohmann/json.hpp>

using nlohmann::json;

class OutputsMessage {
  public:
    explicit OutputsMessage() = default;
    ~OutputsMessage() = default;

    void create_message(const int32_t message_id, const int32_t detection_area, const int16_t delta_x, const int16_t delta_y);

    std::string get_message_as_string() const;

  private:
    json message;
};
