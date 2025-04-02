/**
 * @file drift_lidar_main.cpp
 * @brief Main running loop for drift-lidar application
 */

#include "lidar.hpp"
#include "logging.hpp"
#include "message.hpp"
#include "socket.hpp"

/**
 * @brief Main program loop for drift-lidar application
 *
 * @return 0 on success, -1 on failure
 */
int main()
{
    log_message(INFO, "main(): Starting drift-lidar application");

    const std::string front_lidar_port_name = "/dev/ttyAMA2";
    Lidar front_lidar = Lidar(front_lidar_port_name);

    const std::string down_lidar_port_name = "/dev/ttyAMA3";
    Lidar down_lidar = Lidar(down_lidar_port_name);

    if (!(front_lidar.open_serial() && down_lidar.open_serial())) {
        return -1;
    }

    SocketManager socket;
    if (!socket.initialize_socket()) {
        return -1;
    }

    LidarMessage message;
    int32_t message_id = 0;
    while (1) {
        socket.read_message();
        log_message(INFO, "Received message from server");
        message.create_message(message_id, front_lidar.get_distance(), down_lidar.get_distance());
        socket.send_message(message);
        message_id++;
    }

    front_lidar.close_serial();
    down_lidar.close_serial();

    return 0;
}
