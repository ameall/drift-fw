

#include "lidar.hpp"

#include <string>

int main()
{
    const std::string serial_port_name = "/dev/ttyAMA0";

    Lidar lidar = Lidar(serial_port_name);

    lidar.open_serial();

    fprintf(stdout, "main(): Lidar distance reading: %u\n", lidar.get_distance());

    lidar.close_serial();

    return 0;
}
