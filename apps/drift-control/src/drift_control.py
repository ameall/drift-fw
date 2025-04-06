"""
@file drift_control.py
@brief Drift control class
"""

import asyncio
import math
from mavsdk import System
from mavsdk.offboard import (OffboardError, VelocityBodyYawspeed, VelocityNedYaw)
import time
from typing import Final

# from deployment import GPSDeploy
from drone_controller import DroneController
from log import logger
from server import CAMERA_SOCKET_NAME, LIDAR_SOCKET_NAME, UnixSocketServer


VELOCITY_INCREMENT: Final[float] = 0.25
UPWARD_VELOCITY_OFFSET: Final[float] = -0.05
VELOCITY_RAMP_SLEEP: Final[float] = 0.005

FRAMES_IN_DEADZONE_THRESHOLD: Final[int] = 10
DOWNWARD_LANDING_VELOCITY: Final[float] = 0.5
FORWARD_LANDING_VELOCITY: Final[float] = 3.0

MAX_LIDAR_READING: Final[int] = 1300   # Max distance LiDAR can read
FRONT_LIDAR_HEIGHT_MULTIPLIER: Final[float] = 0.5   # sin(30deg)
FRONT_LIDAR_DISTANCE_MULTIPLIER: Final[float] = 0.866   # cos(30deg)
FRONT_LIDAR_HEIGHT_THRESHOLD: Final[int] = 200   # Distance to switch over to LiDAR-based landing
DROP_THRESHOLD: Final[int] = 300   # Distance to drop payload at
GROUND_FLIGHT_THRESHOLD: Final[int] = 275   # Ground threshold, 275cm -> 9ft
LIDAR_READ_SLEEP: Final[float] = 0.1   # Sleep between LiDAR reads when dropping
DROP_COUNTER: Final[int] = 60


class DriftControl:
    def __init__(self):
        self.drone = System()
        self.controller = DroneController()
        self.camera_server = UnixSocketServer(CAMERA_SOCKET_NAME)
        # self.lidar_server = UnixSocketServer(LIDAR_SOCKET_NAME)
        # self.gps_deployer = GPSDeploy()

        self.camera_server.start_server()
        # self.lidar_server.start_server()


    async def get_heading(self):
        euler = self.drone.telemetry.attitude_euler()
        yaw_deg = math.degrees(euler.yaw)
        return yaw_deg % 360


    async def body_to_NED_velocity(self, foward_velocity, lateral_velocity, heading):
        north_velocity = foward_velocity * math.cos(heading) + lateral_velocity * math.sin(heading)
        east_velocity = foward_velocity * math.sin(heading) + lateral_velocity * math.cos(heading)
        return north_velocity, east_velocity


    async def launch_sequence(self):
        # Use system address serial port for actual drone
        await self.drone.connect("serial:///dev/ttyAMA0:921600")

        logger.info("Waiting for drone to connect...")
        async for state in self.drone.core.connection_state():
            if state.is_connected:
                logger.info(f"-- Connected to drone!")
                break

        logger.info("Waiting for drone to have a global position estimate...")
        async for health in self.drone.telemetry.health():
            if health.is_global_position_ok and health.is_home_position_ok:
                logger.info("-- Global position estimate OK")
                break

        logger.info("-- Arming")
        await self.drone.action.arm()

        logger.info("-- Taking Off")
        await self.drone.action.takeoff()
        await asyncio.sleep(10)

        logger.info("-- Setting initial setpoint")
        await self.drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))

        logger.info("-- Starting offboard")
        try:
            await self.drone.offboard.start()
        except OffboardError as error:
            logger.error(f"Starting offboard mode failed with error code: {error._result.result}")
            logger.error("-- Landing")
            await self.drone.action.land()
            return

        logger.info("-- Go up 1 m/s")
        await self.drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, -1.0, 0.0))
        await asyncio.sleep(4)

        logger.info("-- Hold position for 1s")
        await self.drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, 0.0, 0.0))
        await asyncio.sleep(1)

        await self.drone.offboard.set_velocity_ned(VelocityNedYaw(0.0, 0.0, -0.0, 0.0))


    async def stage_one_flight(self):
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


        async def ramp_velocities_if_needed(updated_x_velocity, updated_y_velocity):
            while velocity_needs_ramping(self.x_velocity, updated_x_velocity) or velocity_needs_ramping(self.y_velocity, updated_y_velocity):
                if velocity_needs_ramping(self.x_velocity, updated_x_velocity):
                    self.x_velocity = ramp_velocity(self.x_velocity, updated_x_velocity)
                if velocity_needs_ramping(self.y_velocity, updated_y_velocity):
                    self.y_velocity = ramp_velocity(self.y_velocity, updated_y_velocity)

                await self.drone.offboard.set_velocity_ned(VelocityNedYaw(self.y_velocity, self.x_velocity, self.z_velocity, 0))
                await asyncio.sleep(VELOCITY_RAMP_SLEEP)


        # async def stage_two_ready():
        #     # Check lidar to see if we've reached our desired height
        #     self.lidar_server.send_message_to_client()
        #     front_height, back_height = self.lidar_server.extract_values_from_lidar_message()
        #     front_height *= FRONT_LIDAR_HEIGHT_MULTIPLIER

        #     # Once we get [THRESHOLD] height from car, switch to LiDAR-based landing
        #     if front_height < FRONT_LIDAR_HEIGHT_THRESHOLD:
        #         self.camera_tracking = False


        frames_in_deadzone = 0

        counter = 0
        # self.lidar_server.accept_client()
        while self.camera_tracking:
            self.camera_server.accept_client()

            delta_x, delta_y, delta_area = self.camera_server.extract_values_from_message()
            # delta_x, delta_y, delta_area = 0, 0, 0
            await asyncio.sleep(0.2)
            logger.info(f"Deltas: {delta_x}, {delta_y}, {delta_area}")
            if delta_x is None:
                logger.error("X IS NONE")
            if delta_y is None:
                logger.error("Y IS NONE")
            if delta_area is None:
                logger.error("AREA IS NONE")
            updated_x_velocity, updated_y_velocity, _ = self.controller.compute_velocity(delta_area, delta_x, delta_y)
            logger.info(f"Setting velocity fwd: {updated_y_velocity}, up: {self.z_velocity}, right: {updated_x_velocity}")

            # # If we reach a certain number of frames in the deadzone, we can assume we've centered the vehicle
            # if updated_y_velocity == 0:
            #     frames_in_deadzone += 1
            #     if frames_in_deadzone >= FRAMES_IN_DEADZONE_THRESHOLD:
            #         logger.info(f"Reached critical number of frames in deadzone, starting descent now")
            #         self.start_landing = True
            # else:
            #     frames_in_deadzone = 0

            # if self.start_landing:
            #     logger.info("Starting landing sequence")
            #     self.z_velocity = DOWNWARD_LANDING_VELOCITY - UPWARD_VELOCITY_OFFSET

            if counter <= DROP_COUNTER:
                await ramp_velocities_if_needed(updated_x_velocity, updated_y_velocity)

            # Use this to set final portion of velocity
            self.x_velocity = updated_x_velocity
            self.y_velocity = updated_y_velocity
            if counter >= DROP_COUNTER:
                logger.info("DESCENDING")
                self.x_velocity = 0
                self.y_velocity = FORWARD_LANDING_VELOCITY
                self.z_velocity = DOWNWARD_LANDING_VELOCITY
                # self.lidar_server.send_message_to_client()
                # front_distance, rear_distance = self.lidar_server.extract_values_from_lidar_message()
                # if rear_distance < DROP_THRESHOLD:
                #     await asyncio.sleep(1)
                #     # self.gps_deployer.setup_servos()
                #     # self.gps_deployer.drop_payload()
                #     self.z_velocity = -DOWNWARD_LANDING_VELOCITY
            await self.drone.offboard.set_velocity_ned(VelocityNedYaw(self.y_velocity, self.x_velocity, self.z_velocity, 0))
            await asyncio.sleep(VELOCITY_RAMP_SLEEP)

            counter += 1
            logger.info(f"Counter: {counter}")
            # await stage_two_ready()

        self.z_velocity = DOWNWARD_LANDING_VELOCITY
        self.y_velocity += DOWNWARD_LANDING_VELOCITY * 1.73
        while True:
            self.lidar_server.send_message_to_client()
            front_distance, rear_distance = self.lidar_server.extract_values_from_lidar_message()
            if rear_distance < DROP_THRESHOLD:
                await asyncio.sleep(1)
                # self.gps_deployer.setup_servos()
                # self.gps_deployer.drop_payload()
                self.z_velocity = -DOWNWARD_LANDING_VELOCITY
            logger.info("DESCENDING 2")

            await self.drone.offboard.set_velocity_ned(VelocityNedYaw(self.y_velocity, self.x_velocity, self.z_velocity, 0))
            await asyncio.sleep(0.1)


    async def stage_two_flight(self):
        async def check_high_enough_off_ground(rear_lidar_reading):
            if rear_lidar_reading < GROUND_FLIGHT_THRESHOLD:
                logger.warning("Hit ground flight threshold, stopping sinking")
                self.z_velocity = 0
                await self.drone.offboard.set_velocity_body(VelocityBodyYawspeed(self.y_velocity, self.x_velocity, self.z_velocity, 0))


        def check_if_reached_drop_height(front_lidar_reading):
            front_lidar_reading *= FRONT_LIDAR_HEIGHT_MULTIPLIER
            if front_lidar_reading < DROP_THRESHOLD:
                logger.warning("Reached drop threshold, holding height")
                self.z_velocity = 0


        # Single read to get distance from car -> Potentially switch this to an average of three
        self.lidar_server.send_message_to_client()
        front_height, back_height = self.lidar_server.extract_values_from_lidar_message()

        self.accurate_front_height = front_height * FRONT_LIDAR_HEIGHT_MULTIPLIER
        self.car_height = back_height - self.accurate_front_height
        self.over_car_threshold = self.car_height * 1 // 4   # If change in reading is quarter-car-height, assume something has changed
        self.over_car = False

        logger.info(f"Front reading: {front_height}, Back reading: {back_height}")
        logger.info(f"Accurate front reading: {self.accurate_front_height}")
        logger.info(f"Car height: {self.car_height}, Over Car Threshold: {self.over_car_threshold}")

        last_back_height = back_height
        while not self.over_car:
            self.lidar_server.send_message_to_client()
            front_height, back_height = self.lidar_server.extract_values_from_lidar_message()

            await check_high_enough_off_ground(back_height)

            await check_if_reached_drop_height(front_height)

            if last_back_height - back_height > self.over_car_threshold:
                self.over_car = True


    async def stage_three_flight(self):
        async def check_if_reached_drop_height(rear_lidar_reading):
            if rear_lidar_reading < DROP_THRESHOLD:
                self.z_velocity = 0
                await self.drone.offboard.set_velocity_body(VelocityBodyYawspeed(self.y_velocity, self.x_velocity, self.z_velocity, 0))


        async def check_if_can_see_past_car(front_height, last_front_height):
            if front_height > last_front_height + self.over_car_threshold:
                # self.gps_deployer.setup_servos()
                # self.gps_deployer.drop_payload()
                self.dropped_payload = True


        self.dropped_payload = False

        last_front_height = 0
        while self.over_car and not self.dropped_payload:
            front_height, back_height = self.lidar_server.extract_values_from_lidar_message()
            front_height *= FRONT_LIDAR_HEIGHT_MULTIPLIER

            await check_if_reached_drop_height(back_height)

            await check_if_can_see_past_car(front_height, last_front_height)

            self.y_velocity += 0.05
            await self.drone.offboard.set_velocity_body(VelocityBodyYawspeed(self.y_velocity, self.x_velocity, self.z_velocity, 0))
            time.sleep(LIDAR_READ_SLEEP)


    async def stop_offboard(self):
        logger.info("-- Stopping offboard")
        try:
            await self.drone.offboard.stop()
        except OffboardError as error:
            logger.error(f"Stopping offboard mode failed with error code: {error._result.result}")


    async def flight_loop(self):
        self.x_velocity = 0
        self.y_velocity = 0
        self.z_velocity = UPWARD_VELOCITY_OFFSET

        self.camera_tracking = True
        self.start_landing = False

        await self.stage_one_flight()

        await self.stage_two_flight()

        await self.stage_three_flight()

        await self.stop_offboard()


async def run():
    drift_drone = DriftControl()

    await drift_drone.launch_sequence()

    await drift_drone.flight_loop()


if __name__ == "__main__":
    asyncio.run(run())
