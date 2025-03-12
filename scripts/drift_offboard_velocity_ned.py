
#!/usr/bin/env python3


import asyncio

from mavsdk import System
from mavsdk.offboard import (OffboardError, VelocityNedYaw)
from src.flight_control_app.drone_controller import DroneController
from src.flight_control_app.pixel_displacement_simulator import CameraAppSimulator


async def run():
    """ Does Offboard control using velocity NED coordinates. """

    drone = System()
    controller = DroneController()
    simulator = CameraAppSimulator()
#   use system address serial port for actual drone
    await drone.connect(system_address="udp://:14540")

    print("Waiting for drone to connect...")
    async for state in drone.core.connection_state():
        if state.is_connected:
            print(f"-- Connected to drone!")
            break

    print("Waiting for drone to have a global position estimate...")
    async for health in drone.telemetry.health():
        if health.is_global_position_ok and health.is_home_position_ok:
            print("-- Global position estimate OK")
            break

    print("-- Arming")
    await drone.action.arm()

    print("-- Taking Off")
    await drone.action.takeoff()
    await asyncio.sleep(15)

    print("-- Setting initial setpoint")
    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))

    print("-- Starting offboard")
    try:
        await drone.offboard.start()
    except OffboardError as error:
        print(f"Starting offboard mode failed with error code: \
              {error._result.result}")
        print("-- Landing")
        await drone.action.land()
        return
    
    print("-- Go up 1 m/s")
    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, -1.0, 0.0))
    await asyncio.sleep(4)

    print("-- Hold position for 2s")
    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    await asyncio.sleep(2)
    
    for i in range(200):

        x, y, z = simulator.run()
        fwd, up, right, _ = controller.compute_velocity(z,y,x)
        await drone.offboard.set_velocity_ned(VelocityNedYaw(fwd, up, right, _))


    # print("-- Go up 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, -1.0, 0.0))
    # await asyncio.sleep(4)

    # print("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

    # print("-- Go North 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(1.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(6)

    # print("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

    # print("-- Go West 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, -1.0, 0.0, 0.0))
    # await asyncio.sleep(6)

    # print("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

    # print("-- Go South 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(-1.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(6)

    # print("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

    # print("-- Go East 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 1.0, 0.0, 0.0))
    # await asyncio.sleep(6)

    # print("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

#    print("-- Turn to face South")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 180.0))
#    await asyncio.sleep(5)

#    print("-- Turn to face North")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(5)

    # print("-- Go down 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 1.0, 0.0))
    # await asyncio.sleep(1)

    # print("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

#    print("--Go North 1 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(1.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    print("--Go North 2 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(2.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    print("--Go North 4 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(4.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    print("--Go North 2 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(2.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    print("--Go North 1 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(1.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    print("-- Hold position for 2s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(2)

    print("-- Stopping offboard")
    try:
        await drone.offboard.stop()
    except OffboardError as error:
        print(f"Stopping offboard mode failed with error code: \
            {error._result.result}")

    return

if __name__ == "__main__":
    # Run the asyncio loop
    asyncio.run(run())
