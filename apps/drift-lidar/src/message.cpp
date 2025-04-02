/**
 * @file message.cpp
 * @brief JSON message manager for sending camera image processing outputs from
 *      this lidar app to the flight control app
 */

#include <unistd.h>

#include "message.hpp"

const std::string MESSAGE_ID_FIELD_NAME = "id";
const std::string FRONT_DISTANCE_FIELD_NAME = "front_lidar_distance";
const std::string DOWN_DISTANCE_FIELD_NAME = "down_lidar_distance";

LidarMessage::LidarMessage() : message_valid(false) {}

void LidarMessage::create_message(const int32_t message_id, const uint16_t front_lidar_distance, const uint16_t down_lidar_distance)
{
    message[MESSAGE_ID_FIELD_NAME] = message_id;
    message[FRONT_DISTANCE_FIELD_NAME] = front_lidar_distance;
    message[DOWN_DISTANCE_FIELD_NAME] = down_lidar_distance;
    message_valid = true;
}

void LidarMessage::clear_message()
{
    message.clear();
    message_valid = false;
}

std::string LidarMessage::get_message_as_string() const
{
    return message.dump();
}

bool LidarMessage::is_message_valid() const
{
    return message_valid;
}

