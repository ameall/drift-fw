#include <mavsdk/component_type.h>
#include <mavsdk/mavsdk.h>
#include <iostream>
#include <chrono>
#include <string>
#include <thread>

int main(int argc, char** argv) {
    mavsdk::Mavsdk mav{mavsdk::Mavsdk::Configuration{mavsdk::ComponentType::GroundStation}};

    // Connection string (e.g., "udp://:14540" for PX4 SITL)
    std::string connection_url = "serial:///dev/ttyAMA0:57600";
    if (argc == 2) {
        connection_url = argv[1];
    }

    // Add connection 
    mavsdk::ConnectionResult connection_result = mav.add_any_connection(connection_url);
    if (connection_result != mavsdk::ConnectionResult::Success) {
        std::cerr << "Connection failed: " << connection_result << std::endl;
        return 1;
    }

    // Wait for system to connect
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Check if any systems are connected
    if (mav.systems().size() == 0) {
        std::cerr << "No systems connected." << std::endl;
        return 1;
    }

    // Get the first connected system
    auto system = mav.systems()[0];

    // Check if the system is connected
    if (!system->is_connected()) {
        std::cerr << "System is not connected." << std::endl;
        return 1;
    }

    // Print connection success message
    std::cout << "Connected to system" << std::endl;

    return 0;
}
