"""
@file drone_controller.py

This class implement as PID controller to determine the appropriate velocity 
for the drone based on the position of the car in the frame.

For this to work, we'll need to fine-tune all of the init parameters.

The compute_velocity method is what we will call inside the while-loop of the
flight_controller application.

In this setup, x is motion left/right
y is motion up/down
z is motion forward/backwards.
"""

from log import logger
from math import copysign
from typing import Final

from pixel_displacement_simulator import CameraAppSimulator


DEFAULT_PROP_GAIN_X: Final[float] = 0.05
DEFAULT_PROP_GAIN_Y: Final[float] = 1.0

DEFAULT_DER_GAIN_X: Final[float] = 0#.05
DEFAULT_DER_GAIN_Y: Final[float] = 0

MAX_VELOCITY: Final[float] = 2.5
MAX_ACCELERATION: Final[float] = 10
FOWARD_VELOCITY_OFFSET: Final[int] = 0

DAMPING_FACTOR: Final[float] = 0.02
MAX_INTEGRAL: Final[int] = 1000
MIN_INTEGRAL: Final[int] = -1000

PID_DEADZONE_WIDTH: Final[int] = 150
PID_DEADZONE_HALF_WIDTH: Final[int] = PID_DEADZONE_WIDTH // 2

PID_DEADZONE_HEIGHT: Final[int] = 0
PID_DEADZONE_HALF_HEIGHT: Final[int] = PID_DEADZONE_HEIGHT // 2


class DroneController:
    def __init__(self) -> None:
        self.kp_x = DEFAULT_PROP_GAIN_X
        self.ki_x = 0
        self.kd_x = DEFAULT_DER_GAIN_X

        self.kp_y = DEFAULT_PROP_GAIN_Y
        self.ki_y = 0
        self.kd_y = DEFAULT_DER_GAIN_Y

        self.prev_x_velocity = 0
        self.prev_y_velocity = 0
        self.prev_z_velocity = 0

        self.prev_x_error = 0
        self.prev_y_error = 0
        self.prev_z_error = 0

        self.integral_x = 0
        self.integral_y = 0
        self.integral_z = 0


    def compute_velocity(self, area, x_error, y_error):
        def clamp(value, max_value, min_value):
            return max(min(value, max_value), min_value)

        def lateral_velocity_clamp(velocity):
            return clamp(velocity, MAX_VELOCITY, -MAX_VELOCITY)

        def forward_velocity_clamp(velocity):
            return clamp(velocity, MAX_VELOCITY * 1.5, 0)

        def acceleration_clamp(current_velocity, previous_velocity):
            return clamp(current_velocity, previous_velocity + MAX_ACCELERATION, previous_velocity - MAX_ACCELERATION)

        def lateral_error_in_deadzone(error):
            return True if (error < PID_DEADZONE_HALF_WIDTH and error > -PID_DEADZONE_HALF_WIDTH) else False

        def forward_error_in_deadzone(error):
            return True if (error < PID_DEADZONE_HALF_HEIGHT and error > -PID_DEADZONE_HALF_HEIGHT) else False

        def offset_error(error, offset):
            if error > offset:
                error -= offset
            elif error < -offset:
                error += offset
            return error

        def calculate_zone_velocity(error):
            if error == 0:
                return 0
            elif error == 1:
                return 1
            elif error == 2:
                return 1.5
            elif error == 3:
                return 2
            elif error == 4:
                return 2.5
            else:
                return 0

        # y_error = -y_error

        x_in_deadzone = lateral_error_in_deadzone(x_error)
        y_in_deadzone = forward_error_in_deadzone(y_error)

        x_error = offset_error(x_error, PID_DEADZONE_HALF_WIDTH)
        y_error = offset_error(y_error, PID_DEADZONE_HALF_HEIGHT)

        derivative_x = x_error - self.prev_x_error
        derivative_y = y_error - self.prev_y_error

        # Compute velocities
        x_velocity = (self.kp_x * x_error) + (self.ki_x * self.integral_x) + (self.kd_x * derivative_x)
        # y_velocity = (self.kp_y * y_error) + (self.ki_y * self.integral_y) + (self.kd_y * derivative_y)
        y_velocity = calculate_zone_velocity(y_error)

        # Cap velocities to within a set bound
        x_velocity = lateral_velocity_clamp(x_velocity)
        y_velocity = forward_velocity_clamp(y_velocity)

        # Cap change in velocity to within a set bound
        x_velocity = acceleration_clamp(x_velocity, self.prev_x_velocity)
        y_velocity = acceleration_clamp(y_velocity, self.prev_y_velocity)

        self.prev_x_velocity = x_velocity
        self.prev_y_velocity = y_velocity

        self.prev_x_error = x_error
        self.prev_y_error = y_error

        if x_in_deadzone:
            logger.info("X DEADZONE")
            x_velocity = 0
        if y_in_deadzone:
            logger.info("Y DEADZONE")
            y_velocity = 0

        return x_velocity, y_velocity + FOWARD_VELOCITY_OFFSET, 0


def main():
    # Example usage
    controller = DroneController()
    simulator = CameraAppSimulator(50, -25, 2500)
    for _ in range(100):  # Simulating 10 frames
        x, y, z = simulator.run()
        fwd, up, right = controller.compute_velocity(z,y,x)
        logger.info("Velocity Vector:", round(fwd,ndigits=3), round(up,ndigits=3), right)


if __name__ == "__main__":
    main()
