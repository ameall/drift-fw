

#include "lidar.hpp"
#include "message.hpp"
#include "logging.hpp"
#include "socket.hpp"

#include <string>

int main()
{
    const std::string serial_port_name = "/dev/ttyAMA0";
    Lidar lidar = Lidar(serial_port_name);

    if (!lidar.open_serial()) {
        return -1;
    }

    LidarSocket lidar_socket;
    if (!lidar_socket.initialize_socket()) {
        return -1;
    }

    int message_id = 0;

    while (true) {
        uint16_t distance = lidar.get_distance();

        fprintf(stdout, "main(): Lidar distance reading: %u\n", distance);

        LidarMessage message;
        message.create_message(message_id++, distance)

        if (!lidar_socket.send_distance(distance)) {
            break;
        }
        usleep(100000); // Send data every 100ms
    }

    lidar.close_serial();
    return 0;

}
