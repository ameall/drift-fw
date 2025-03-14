
#!/usr/bin/env python3


import asyncio
import time
from mavsdk import System
from mavsdk.offboard import (OffboardError, VelocityNedYaw)

class DroneController:
    def __init__(self, kp_x=0.03, ki_x=0, kd_x=0.03,
                 kp_y=0.03, ki_y=0, kd_y=0.03,
                 kp_z=0.00025, ki_z=0, kd_z=0.0025,
                 initial_target_area=20000, max_area_growth=100):
        self.kp_x, self.ki_x, self.kd_x = kp_x, ki_x, kd_x
        self.kp_y, self.ki_y, self.kd_y = kp_y, ki_y, kd_y
        self.kp_z, self.ki_z, self.kd_z = kp_z, ki_z, kd_z

        self.target_area = initial_target_area
        self.max_area_growth = max_area_growth

        self.prev_x_error = 0
        self.prev_y_error = 0
        self.prev_z_error = 0

        self.integral_x = 0
        self.integral_y = 0
        self.integral_z = 0

        self.max_velocity = 5.0

    def compute_velocity(self, area, x, y):
        # Dynamic area growth adjustment with damping
        distance_error = self.target_area - area
        area_growth_rate = min(self.max_area_growth, abs(distance_error) * 0.02)
        self.target_area += area_growth_rate * (1 if distance_error > 0 else -1)

        # Errors
        x_error = -x
        y_error = -y
        z_error = self.target_area - area

        # PID Components
        self.integral_x = max(min(self.integral_x + x_error, 1000), -1000)
        self.integral_y = max(min(self.integral_y + y_error, 1000), -1000)
        self.integral_z = max(min(self.integral_z + z_error, 1000), -1000)

        derivative_x = x_error - self.prev_x_error
        derivative_y = y_error - self.prev_y_error
        derivative_z = z_error - self.prev_z_error

        # Compute velocities
        vel_x = (self.kp_x * x_error) + (self.ki_x * self.integral_x) + (self.kd_x * derivative_x)
        vel_y = (self.kp_y * y_error) + (self.ki_y * self.integral_y) + (self.kd_y * derivative_y)
        vel_z = (self.kp_z * z_error) + (self.ki_z * self.integral_z) + (self.kd_z * derivative_z)

        # Cap velocities to avoid unstable behavior
        vel_x = max(min(vel_x, self.max_velocity), -self.max_velocity)
        vel_y = max(min(vel_y, self.max_velocity), -self.max_velocity)
        vel_z = max(min(vel_z, self.max_velocity), -self.max_velocity)

        # Update previous errors
        self.prev_x_error = x_error
        self.prev_y_error = y_error
        self.prev_z_error = z_error

        return vel_z, vel_x, vel_y, 0

class CameraAppSimulator:
    def __init__(self, x=10, y=10, z=4000, frame_rate=10):
        self.delta_x = x
        self.delta_y = y
        self.delta_z = z
        self.frame_rate = frame_rate
        self.timestep = 50

    def update_parameters(self):
        self.timestep = 50 if self.timestep == -30 else self.timestep

        # Balanced oscillation for left/right and up/down
        if self.timestep > 0 :
            self.delta_x += 5 
            self.delta_y += 5
        else :
            self.delta_x -= 5
            self.delta_y -= 5

        self.delta_z = min(self.delta_z + 50, 20000)  # Slow increase to stabilize
        self.timestep -= 1

    def run(self):
        self.update_parameters()
        time.sleep(1 / self.frame_rate)
        return self.delta_x, self.delta_y, self.delta_z

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
