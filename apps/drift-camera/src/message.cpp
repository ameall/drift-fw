/**
 * @file message.cpp
 * @brief JSON message manager for sending camera image processing outputs from
 *      this camera app to the flight control app
 */

#include "message.hpp"

const std::string MESSAGE_ID_FIELD_NAME = "id";
const std::string DETECTION_AREA_FIELD_NAME = "det_area";
const std::string DELTA_X_FIELD_NAME = "x_off";
const std::string DELTA_Y_FIELD_NAME = "y_off";

OutputsMessage::OutputsMessage() : message_valid(false) {}

void OutputsMessage::create_message(const int32_t message_id, const int32_t detection_area, const int16_t delta_x, const int16_t delta_y)
{
    message[MESSAGE_ID_FIELD_NAME] = message_id;
    message[DETECTION_AREA_FIELD_NAME] = detection_area;
    message[DELTA_X_FIELD_NAME] = delta_x;
    message[DELTA_Y_FIELD_NAME] = delta_y;

    message_valid = true;
}

void OutputsMessage::clear_message()
{
    message.clear();
    message_valid = false;
}

std::string OutputsMessage::get_message_as_string() const
{
    return message.dump();
}

bool OutputsMessage::is_message_valid() const
{
    return message_valid;
}
