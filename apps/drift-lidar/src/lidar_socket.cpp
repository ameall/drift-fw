#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "message.hpp"
#include "lidar.hpp"
#include "logging.hpp"
#include "socket.hpp"

const std::string DEFAULT_SOCKET_DIR = "/run/";

class SocketManager {
public:
    SocketManager(const std::string socket_name) : socket_fd(-1), socket_path(get_socket_path(socket_name)) {}
    ~SocketManager() { close_socket(); }

    static std::string get_socket_path(const std::string& socket_name = DEFAULT_SOCKET_NAME)
    {
        const char* xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");

        if (xdg_runtime_dir && *xdg_runtime_dir) {
            return std::string(xdg_runtime_dir) + "/" + socket_name;
        }
        else {
            return DEFAULT_SOCKET_DIR + socket_name;
        }
    }

    bool initialize_socket()
    {
        socket_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
        if (socket_fd == -1) {
            log_message(ERROR, "SocketManager::initialize_socket(): Failed to create socket");
            return false;
        }

        memset(&socket_address, 0, sizeof(socket_address));
        socket_address.sun_family = AF_UNIX;
        strncpy(socket_address.sun_path, socket_path.c_str(), sizeof(socket_address.sun_path) - 1);

        if (connect(socket_fd, reinterpret_cast<const struct sockaddr*>(&socket_address), sizeof(socket_address)) == -1) {
            log_message(ERROR, "SocketManager::initialize_socket(): Failed to connect to socket");
            return false;
        }

        return true;
    }

    bool send_lidar_data(LidarMessage& message)
    {
        if (socket_fd == -1) {
            log_message(ERROR, "SocketManager::send_lidar_data(): Socket not open, could not send data");
            return false;
        }

        if (!message.is_message_valid()) {
            log_message(ERROR, "SocketManager::send_message(): JSON message is not valid");
            return false;
        }

        std::string message = "{\"distance\": " + std::to_string(message.distance) + "}";

        if (send(socket_fd, message.c_str(), message.size(), 0) == -1) {
            log_message(ERROR, "SocketManager::send_lidar_data(): Failed to send data on socket");
            return false;
        }

        return true;
    }

    

    bool close_socket()
    {
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

private:
    int socket_fd;
    struct sockaddr_un socket_address;
};