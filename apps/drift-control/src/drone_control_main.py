"""
@file drone_control_test.py
@brief Drone control test script
"""

import asyncio
from mavsdk import System
from mavsdk.offboard import (OffboardError, VelocityNedYaw)
import time

from drone_controller import DroneController
from log import logger
from pixel_displacement_simulator import CameraAppSimulator
from server import UnixSocketServer


async def run():
    """ Does Offboard control using velocity NED coordinates. """
    controller = DroneController()
    server = UnixSocketServer()
    server.start_server()
    # while True:
    #     server.accept_client()
    #     x, y, area = server.extract_values_from_message()
    #     fwd, up, right, _ = controller.compute_velocity(area, x, y)
    #     logger.info("");
    #     logger.info(f"Velocity fwd: {fwd}, up: {up}, right: {right}")
    #     logger.info("");

    drone = System()
    controller = DroneController()
    simulator = CameraAppSimulator()
    # Use system address serial port for actual drone
    await drone.connect("serial:///dev/ttyAMA0:921600")

    logger.info("Waiting for drone to connect...")
    async for state in drone.core.connection_state():
        if state.is_connected:
            logger.info(f"-- Connected to drone!")
            break

    logger.info("Waiting for drone to have a global position estimate...")
    async for health in drone.telemetry.health():
        if health.is_global_position_ok and health.is_home_position_ok:
            logger.info("-- Global position estimate OK")
            break

    logger.info("-- Arming")
    await drone.action.arm()

    logger.info("-- Taking Off")
    await drone.action.takeoff()
    await asyncio.sleep(15)

    logger.info("-- Setting initial setpoint")
    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))

    logger.info("-- Starting offboard")
    try:
        await drone.offboard.start()
    except OffboardError as error:
        logger.error(f"Starting offboard mode failed with error code: \
              {error._result.result}")
        logger.error("-- Landing")
        await drone.action.land()
        return

    logger.info("-- Go up 1 m/s")
    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, -1.0, 0.0))
    await asyncio.sleep(4)

    logger.info("-- Hold position for 2s")
    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    await asyncio.sleep(3)

    while True:
        server.accept_client()
        x, y, area = server.extract_values_from_message()
        fwd, up, right, _ = controller.compute_velocity(area, x, y)
        logger.info(f"Velocity fwd: {fwd}, up: {up}, right: {right}")
        await drone.offboard.set_velocity_ned(VelocityNedYaw(fwd, -up, right, _))

    ''' # For landing when switching over to Lidar

    Landing = True

    while Landing:
        distance = server.extract_values_from_message()'
        # figure this out

    '''
        
        
    # for i in range(200):

    #     x, y, z = simulator.run()
    #     fwd, up, right, _ = controller.compute_velocity(z,y,x)
    #     await drone.offboard.set_velocity_ned(VelocityNedYaw(fwd, up, right, _))


    # logger.info("-- Go up 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, -1.0, 0.0))
    # await asyncio.sleep(4)

    # logger.info("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

    # logger.info("-- Go North 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(1.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(6)

    # logger.info("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

    # logger.info("-- Go West 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, -1.0, 0.0, 0.0))
    # await asyncio.sleep(6)

    # logger.info("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

    # logger.info("-- Go South 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(-1.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(6)

    # logger.info("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

    # logger.info("-- Go East 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 1.0, 0.0, 0.0))
    # await asyncio.sleep(6)

    # logger.info("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

#    logger.info("-- Turn to face South")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 180.0))
#    await asyncio.sleep(5)

#    logger.info("-- Turn to face North")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(5)

    # logger.info("-- Go down 1 m/s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 1.0, 0.0))
    # await asyncio.sleep(1)

    # logger.info("-- Hold position for 2s")
    # await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
    # await asyncio.sleep(2)

#    logger.info("--Go North 1 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(1.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    logger.info("--Go North 2 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(2.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    logger.info("--Go North 4 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(4.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    logger.info("--Go North 2 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(2.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    logger.info("--Go North 1 m/s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(1.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(1)

#    logger.info("-- Hold position for 2s")
#    await drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
#    await asyncio.sleep(2)

    logger.info("-- Stopping offboard")
    try:
        await drone.offboard.stop()
    except OffboardError as error:
        logger.error(f"Stopping offboard mode failed with error code: \
            {error._result.result}")

    return


if __name__ == "__main__":
    # controller = DroneController()
    # server = UnixSocketServer()
    # server.start_server()
    # server.accept_client()

    # while True:
    #     x, y, area = server.extract_values_from_message()
    #     if x == -1 and y == -1 and area == -1:
    #         server.accept_client()
    #         continue
    #     fwd, up, right, _ = controller.compute_velocity(area,y,x)
    #     logger.info(f"Velocity fwd: {fwd} up: {up}, right {right}")

    # Run the asyncio loop
    asyncio.run(run())
