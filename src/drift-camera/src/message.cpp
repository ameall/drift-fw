/**
 * @file message.cpp
 * @brief JSON message manager for sending camera image processing outputs from
 *      this camera app to the flight control app
 */

#include <nlohmann/json.hpp>

#include "message.hpp"

const std::string MESSAGE_ID_FIELD_NAME = "id";
const std::string DETECTION_AREA_FIELD_NAME = "det_area";
const std::string DELTA_X_FIELD_NAME = "x_off";
const std::string DELTA_Y_FIELD_NAME = "y_off";

void OutputsMessage::create_message(const int32_t message_id, const int32_t detection_area, const int16_t delta_x, const int16_t delta_y)
{
    message[MESSAGE_ID_FIELD_NAME] = message_id;
    message[DETECTION_AREA_FIELD_NAME] = detection_area;
    message[DELTA_X_FIELD_NAME] = delta_x;
    message[DELTA_Y_FIELD_NAME] = delta_y;
}

std::string OutputsMessage::get_message_as_string() const
{
    return message.dump();
}
