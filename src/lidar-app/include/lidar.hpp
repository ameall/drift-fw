/**
 * @file lidar.hpp
 * @brief LiDAR class definition for DRIFT Drone
 */

#pragma once

// @cond
#include <cstdint>
#include <string>
// @endcond

constexpr uint32_t BAUD_RATE = 115200u;
constexpr uint8_t LIDAR_PACKAGE_SIZE = 9u;

class Lidar {
  public:
    Lidar(const std::string &port);
    ~Lidar() = default;

    bool open_serial();

    uint16_t get_distance();

    void close_serial();

  private:
    int32_t serial_port = -1;

    std::string port_name = "";

    uint8_t lidar_reading[LIDAR_PACKAGE_SIZE];
};
