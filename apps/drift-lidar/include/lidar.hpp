/**
 * @file lidar.hpp
 * @brief LiDAR class definition for DRIFT Drone
 */

#pragma once

#include <cstdint>
#include <string>

constexpr int8_t INVALID_SERIAL_PORT = -1;
constexpr uint8_t LIDAR_PACKAGE_SIZE = 9u;

class Lidar {
  public:
    Lidar(const std::string &port);
    ~Lidar() = default;

    /**
     * @brief Opens the serial port specified by port_name
     *
     * @return true if the serial port is successfully opened and configured
     * @return false if the serial port is unable to be connected to/configured
     */
    bool open_serial();

    /**
     * @brief Gets a distance reading from the LiDAR sensor
     *
     * @return The distance reading in cm, or 0 if the distance cannot be read
     */
    uint16_t get_distance();

    /**
     * @brief Closes the serial port associated with the LiDAR sensor
     */
    void close_serial();

  private:
    int32_t serial_port = INVALID_SERIAL_PORT;

    std::string port_name = "";

    /**
     * @brief LiDAR reading; for a breakdown of the package contents refer to:
     *      https://cdn.sparkfun.com/assets/2/b/0/3/8/TFmini_Plus-01-A02-Datasheet_EN.pdf
     */
    uint8_t lidar_reading[LIDAR_PACKAGE_SIZE];
};
