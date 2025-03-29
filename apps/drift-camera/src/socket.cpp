/**
 * @file socket.cpp
 * @brief Socket manager for sending a structured message from this camera app
 *      to the flight control app
 */

#include <unistd.h>

#include "logging.hpp"
#include "socket.hpp"

const std::string DEFAULT_SOCKET_DIR = "/run/";

/**
 * @brief Creates a full path to the UNIX socket based on whether the system
 *      uses XDG
 *
 * @param socket_name Filename for the socket
 * @return Full path to the UNIX socket
 */
static std::string get_socket_path(const std::string &socket_name = DEFAULT_SOCKET_NAME)
{
    const char* xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");

    if (xdg_runtime_dir && *xdg_runtime_dir) {
        return std::string(xdg_runtime_dir) + "/" + socket_name;
    } else {
        return DEFAULT_SOCKET_DIR + socket_name;
    }
}

SocketManager::SocketManager(const std::string socket_name) : socket_fd(-1), socket_path(get_socket_path(socket_name)) {}

SocketManager::~SocketManager()
{
    close_socket();
}

bool SocketManager::initialize_socket()
{
    socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (socket_fd == -1) {
        log_message(ERROR, "SocketManager::connect(): Failed to create socket");
        return false;
    }

    memset(&socket_address, 0, sizeof(socket_address));

    socket_address.sun_family = AF_UNIX;
    strncpy(socket_address.sun_path, socket_path.c_str(), sizeof(socket_address.sun_path) - 1);

    if (connect(socket_fd, reinterpret_cast<const struct sockaddr *>(&socket_address), sizeof(socket_address)) == -1) {
        log_message(ERROR, "SocketManager::connect(): Failed to connect to socket");
        return false;
    }

    return true;
}

bool SocketManager::send_message(OutputsMessage &message)
{
    if (socket_fd == -1) {
        log_message(ERROR, "SocketManager::send_message(): Socket not open, could not send message");
        return false;
    }

    if (!message.is_message_valid()) {
        log_message(ERROR, "SocketManager::send_message(): JSON message is not valid");
        return false;
    }

    if (send(socket_fd, message.get_message_as_string().c_str(), message.get_message_as_string().size(), 0) == -1) {
        log_message(ERROR, "SocketManager::send_message(): Failed to send message on socket");
        return false;
    }
    message.clear_message();

    return true;
}

bool SocketManager::close_socket()
{
    log_message(INFO, "SocketManager::close_socket(): Closing socket");

    if (socket_fd == -1) {
        log_message(INFO, "SocketManager::close_socket(): No socket is open");
        return true;
    }

    if (close(socket_fd) == -1) {
        log_message(ERROR, "SocketManager::close_socket(): Unable to close socket");
        return false;
    }
    socket_fd = -1;

    return true;
}
