"""
@file drone_control_test.py
@brief Drone control test script
"""

import asyncio
import time
from mavsdk import System
from mavsdk.offboard import (OffboardError, VelocityBodyYawspeed)

from drone_controller import DroneController
from log import logger
from pixel_displacement_simulator import CameraAppSimulator
from server import UnixSocketServer


async def run():
    """ Does Offboard control using velocity body coordinates. """
    # ======= Camera App Simulator ======= #
    # controller = DroneController()
    # server = UnixSocketServer()
    # server.start_server()
    # while True:
    #     server.accept_client()
    #     x, y, area = server.extract_values_from_message()
    #     fwd, up, right = controller.compute_velocity(area, x, y)
    #     logger.info("");
    #     logger.info(f"Velocity fwd: {fwd}, up: {up}, right: {right}")
    #     logger.info("");

    # ======= Main Flight Loop ======= #
    drone = System()
    controller = DroneController()
    server = UnixSocketServer()
    server.start_server()
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
    await drone.offboard.set_velocity_body(VelocityBodyYawspeed(0.0, 0.0, 0.0, 0.0))

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
    await drone.offboard.set_velocity_body(VelocityBodyYawspeed(0.0, 0.0, -1.0, 0.0))
    await asyncio.sleep(6)

    logger.info("-- Hold position for 2s")
    await drone.offboard.set_velocity_body(VelocityBodyYawspeed(0.0, 0.0, 0.0, 0.0))
    await asyncio.sleep(3)

    x_velocity = 0
    while True:
        server.accept_client()
        x, y, area = server.extract_values_from_message()
        right, fwd, up = controller.compute_velocity(area, x, y)
        logger.info(f"Setting velocity fwd: {fwd}, up: {up}, right: {right}")
        if (x_velocity - right > 0):
            while (x_velocity - right > 0):
                x_velocity = x_velocity - 0.2
                await drone.offboard.set_velocity_body(VelocityBodyYawspeed(fwd, x_velocity, -0.05, 0))
                time.sleep(0.05)
        if (x_velocity - right < 0):
            while (x_velocity - right < 0):
                x_velocity = x_velocity + 0.2
                await drone.offboard.set_velocity_body(VelocityBodyYawspeed(fwd, x_velocity, -0.05, 0))
                time.sleep(0.05)

    logger.info("-- Stopping offboard")
    try:
        await drone.offboard.stop()
    except OffboardError as error:
        logger.error(f"Stopping offboard mode failed with error code: \
            {error._result.result}")

    return


if __name__ == "__main__":
    # ======= Camera App Simulator ======= #
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

    # ======= Main Flight Loop ======= #
    asyncio.run(run())
