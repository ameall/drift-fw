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

const std::string DEFAULT_SOCKET_NAME = "DRIFT.sock";

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
    bool send_message(const OutputsMessage &message);

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
