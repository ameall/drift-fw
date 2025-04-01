/**
 * @file socket.hpp
 * @brief SocketManager class declaration
 */

#pragma once

#include <cstdint>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "message.hpp"

const std::string DEFAULT_SOCKET_NAME = "DRIFT.sock";

/**
 * @class SocketManager
 * @brief Manages the UNIX socket for sending extracted detection info from the
 *      camera application to the flight control application
 */
class SocketManager {
  public:
    /**
     * @brief Creates a socket object
     *
     * @param socket_name Socket name
     */
    explicit SocketManager(const std::string socket_name = DEFAULT_SOCKET_NAME);

    /**
     * @brief Ensures that the socket is closed when the object goes out of
     *      scope
     */
    ~SocketManager();

    // Deleting copy and move constructors
    SocketManager(const SocketManager&) = delete;
    SocketManager& operator=(const SocketManager&) = delete;
    SocketManager(SocketManager&&) = delete;
    SocketManager& operator=(SocketManager&&) = delete;

    /**
     * @brief Creates the actual socket and connects to the server
     *
     * @return true if the socket is successfully created; false otherwise
     */
    bool initialize_socket();

    /**
     * @brief Sends a string message to the server
     *
     * @param message Camera outputs object containing message to send
     * @return true if the message is able to be sent; false otherwise 
     */
    bool send_message(OutputsMessage &message);

    /**
     * @brief Closes the socket, if it is open
     *
     * @return true if the socket is successfully closed; false otherwise
     */
    bool close_socket();

  private:
    int32_t socket_fd;
    std::string socket_path;
    struct sockaddr_un socket_address;
};
