/**
 * @file drift_lidar_main.cpp
 * @brief Main running loop for drift-lidar application
 */

#include "lidar.hpp"
#include "logging.hpp"
#include "socket.hpp"

/**
 * @brief Main program loop for drift-lidar application
 *
 * @return 0 on success, -1 on failure
 */
int main()
{
    log_message(INFO, "main(): Starting drift-lidar application");

    const std::string serial_port_name = "/dev/ttyAMA0";
    Lidar lidar = Lidar(serial_port_name);

    if (!lidar.open_serial()) {
        return -1;
    }

    SocketManager socket;
    if (!socket.initialize_socket()) {
        return -1;
    }

    while (1) {
        // Add logic to allow flight control app to tell camera app to stop
    }

    lidar.close_serial();

    return 0;
}
