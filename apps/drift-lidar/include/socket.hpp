/**
 * @file socket.hpp
 * @brief Socket manager for sending a structured message from this camera app
 *      to the flight control app
 */

#pragma once

#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "message.hpp"

const std::string DEFAULT_SOCKET_NAME = "DRIFT_LIDAR.sock";

class SocketManager {
public:
    SocketManager(const std::string socket_name = DEFAULT_SOCKET_NAME);
    ~SocketManager();

    bool initialize_socket();
    bool send_lidar_data(LidarMessage &message);
    bool close_socket();

private:
    int socket_fd;
    struct sockaddr_un socket_address;
    std::string socket_path;
};

#endif // SOCKET_HPP
