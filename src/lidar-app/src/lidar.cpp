/**
 * @file lidar.cpp
 * @brief Lidar class for DRIFT Drone
 */

#include <cstdint>
#include <fcntl.h>
#include <iterator>
#include <termios.h>
#include <unistd.h>

#include "lidar.hpp"

constexpr uint8_t NUM_BITS_PER_BYTE = 8u;
constexpr uint16_t BAUD_RATE = B115200;
constexpr uint8_t FRAME_HEADER_VALID_BYTE = 0x59;

Lidar::Lidar(const std::string &port) : port_name(port) {};

bool Lidar::open_serial()
{
    serial_port = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_port < 0) {
        fprintf(stderr, "Lidar::open_serial(): Error opening serial port %s\n", port_name.c_str());
        return false;
    }

    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0) {
        fprintf(stderr, "Lidar::open_serial(): Failed to get serial port parameters\n");
        return false;
    }

    cfsetospeed(&tty, BAUD_RATE);
    cfsetispeed(&tty, BAUD_RATE);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;   // 8-bit characters
    tty.c_iflag &= ~IGNBRK;                       // disable break processing
    tty.c_lflag = 0;                              // no signaling chars, no echo, no canonical processing
    tty.c_oflag = 0;                              // no remapping, no delays
    tty.c_cc[VMIN] = 9;                           // read doesn't block
    tty.c_cc[VTIME] = 1;                          // 0.1 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);        // ignore modem controls and enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Lidar::open_serial(): Failed to set serial port parameters\n");
        return false;
    }

    return true;
}

uint16_t Lidar::get_distance()
{
    if (serial_port == -1) {
        fprintf(stderr, "Lidar::get_distance(): Serial port not open\n");
        return 0;
    }

    ssize_t bytes_read = read(serial_port, lidar_reading, std::size(lidar_reading));

    bool valid_reading = bytes_read == LIDAR_PACKAGE_SIZE
                         && lidar_reading[0] == FRAME_HEADER_VALID_BYTE
                         && lidar_reading[1] == FRAME_HEADER_VALID_BYTE;
    if (!valid_reading) {
        fprintf(stderr, "Lidar::get_distance(): Invalid reading from the LiDAR sensor\n");
        return 0;
    }

    return static_cast<int16_t>((lidar_reading[3] << NUM_BITS_PER_BYTE) + lidar_reading[2]);
}

void Lidar::close_serial()
{
    if (serial_port == INVALID_SERIAL_PORT) {
        fprintf(stdout, "Lidar::close_serial(): Serial port is already closed\n");
    }

    close(serial_port);
}
