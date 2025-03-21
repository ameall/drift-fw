#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "message.hpp"

const std::string MESSAGE_ID_FIELD_NAME = "id";
const std::string DISTANCE_FIELD_NAME = "distance";

LidarMessage::LidarMessage() : message_valid(false) {}

void LidarMessage::create_message(const int32_t message_id, const uint16_t distance) {
    message[MESSAGE_ID_FIELD_NAME] = message_id;
    message[DISTANCE_FIELD_NAME] = distance;
    message_valid = true;
}

void LidarMessage::clear_message() {
    message.clear();
    message_valid = false;
}

std::string LidarMessage::get_message_as_string() const {
    return message.dump();
}

bool LidarMessage::is_message_valid() const {
    return message_valid;
}

