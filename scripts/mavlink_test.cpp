#include <cstdlib>
#include <mavsdk/connection_result.h>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>
#include <iostream>
#include <chrono>
#include <mavsdk/system.h>
#include <memory>
#include <ostream>
#include <string>
#include <thread>

void handle_connection_error(mavsdk::ConnectionResult result)
{
    if (result != mavsdk::ConnectionResult::Success) {
        std::cerr << "Connection failed: " << std::endl;
        exit(EXIT_FAILURE);
    }
}

void handle_action_result(mavsdk::Action::Result result)
{
    if (result != mavsdk::Action::Result::Success) {
        std::cerr << "Action failed" << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main()
{
    mavsdk::Mavsdk mavsdk;

    const std::string connection_url = "serial://dev/ttyAMA0::57600";
    mavsdk::ConnectionResult connectcion_result = mavsdk.add_any_connection(connection_url);
    handle_connection_error(connectcion_result);

    std::cout << "Waiting for system discovery..." << std::endl;
    while (mavsdk.systems().empty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::shared_ptr<mavsdk::System> system = mavsdk.systems().at(0);
    std::cout << "System connected!" << std::endl;
}
