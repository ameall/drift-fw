"""
@file drone_control_test.py
@brief Drone control test script
"""

import asyncio
import time
from mavsdk import System
from mavsdk.offboard import (OffboardError, VelocityBodyYawspeed)
from typing import Final

from deployment import GPSDeploy
from drone_controller import DroneController
from log import logger
from pixel_displacement_simulator import CameraAppSimulator
from server import CAMEAR_SOCKET_NAME, CAMERA_SOCKET_NAME, LIDAR_SOCKET_NAME, UnixSocketServer


VELOCITY_INCREMENT: Final[float] = 0.25
UPWARD_VELOCITY_OFFSET: Final[float] = -0.2
VELOCITY_RAMP_SLEEP: Final[float] = 0.0025

def ramp_velocity(current_velocity, updated_velocity):
    if (updated_velocity > current_velocity):
        while (updated_velocity - current_velocity > VELOCITY_INCREMENT):
            current_velocity = current_velocity + VELOCITY_INCREMENT
            return current_velocity
    if (updated_velocity < current_velocity):
        while (updated_velocity - current_velocity < VELOCITY_INCREMENT):
            current_velocity = current_velocity - VELOCITY_INCREMENT
            return current_velocity

def velocity_needs_ramping(current_velocity, updated_velocity):
    return abs(updated_velocity - current_velocity) > VELOCITY_INCREMENT


async def run():
    """ Does Offboard control using velocity body coordinates. """
    # ======= Camera App Simulator ======= #
    # controller = DroneController()
    # camera_server = UnixSocketServer()
    # camera_server.start_server()
    # while True:
    #     camera_server.accept_client()
    #     x, y, area = camera_server.extract_values_from_message()
    #     fwd, up, right = controller.compute_velocity(area, x, y)
    #     logger.info("");
    #     logger.info(f"Velocity fwd: {fwd}, up: {up}, right: {right}")
    #     logger.info("");

    # ======= Main Flight Loop ======= #
    drone = System()
    controller = DroneController()
    camera_server = UnixSocketServer(CAMERA_SOCKET_NAME)
    camera_server.start_server()
    lidar_server = UnixSocketServer(LIDAR_SOCKET_NAME)
    lidar_server.start_server()
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

    lidar_server.accept_client()
    gps_deployer = GPSDeploy()
    while (True):
        lidar_server.send_message_to_client()
        front_distance, back_distance = lidar_server.extract_values_from_lidar_message()
        if (back_distance < 20 and not back_distance == 0):
            gps_deployer.setup_servos()
            gps_deployer.drop_payload()
            break

    x_velocity = 0
    y_velocity = 0
    z_velocity = 0
    frames_in_deadzone = 0
    while (frames_in_deadzone > 10):
        camera_server.accept_client()
        x, y, area = camera_server.extract_values_from_message()
        updated_x_velocity, updated_y_velocity, updated_z_velocity = controller.compute_velocity(area, x, y)
        logger.info(f"Setting velocity fwd: {updated_y_velocity}, up: {updated_z_velocity}, right: {updated_x_velocity}")

        while (velocity_needs_ramping(x_velocity, updated_x_velocity) or velocity_needs_ramping(y_velocity, updated_y_velocity) or velocity_needs_ramping(z_velocity, updated_z_velocity)) :
            if velocity_needs_ramping(x_velocity, updated_x_velocity):
                x_velocity = ramp_velocity(x_velocity, updated_x_velocity)
            if velocity_needs_ramping(y_velocity, updated_y_velocity):
                y_velocity = ramp_velocity(y_velocity, updated_y_velocity)
            if velocity_needs_ramping(z_velocity, updated_z_velocity):
                z_velocity = ramp_velocity(z_velocity, updated_z_velocity)
            await drone.offboard.set_velocity_body(VelocityBodyYawspeed(y_velocity, x_velocity, UPWARD_VELOCITY_OFFSET, 0))
            time.sleep(VELOCITY_RAMP_SLEEP)

        x_velocity = updated_x_velocity
        y_velocity = updated_y_velocity
        z_velocity = updated_z_velocity
        await drone.offboard.set_velocity_body(VelocityBodyYawspeed(y_velocity, x_velocity, UPWARD_VELOCITY_OFFSET, 0))
        time.sleep(VELOCITY_RAMP_SLEEP)


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
    # camera_server = UnixSocketServer()
    # camera_server.start_server()
    # camera_server.accept_client()

    # while True:
    #     x, y, area = camera_server.extract_values_from_message()
    #     if x == -1 and y == -1 and area == -1:
    #         camera_server.accept_client()
    #         continue
    #     fwd, up, right, _ = controller.compute_velocity(area,y,x)
    #     logger.info(f"Velocity fwd: {fwd} up: {up}, right {right}")

    # ======= Main Flight Loop ======= #
    asyncio.run(run())
