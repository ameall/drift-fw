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


DEFAULT_PROP_GAIN_X: Final[float] = 0
DEFAULT_PROP_GAIN_Y: Final[float] = 0
DEFAULT_PROP_GAIN_Z: Final[float] = 0

DEFAULT_INT_GAIN_X: Final[int] = 0
DEFAULT_INT_GAIN_Y: Final[int] = 0
DEFAULT_INT_GAIN_Z: Final[int] = 0

DEFAULT_DER_GAIN_X: Final[float] = 0
DEFAULT_DER_GAIN_Y: Final[float] = 0
DEFAULT_DER_GAIN_Z: Final[float] = 0

INITIAL_TARGET_AREA: Final[int] = 2000
MAX_AREA_GROWTH: Final[int] = 100

MAX_VELOCITY: Final[int] = 2
MAX_ACCELERATION: Final[float] = 0.4

DAMPING_FACTOR: Final[float] = 0.02
MAX_INTEGRAL: Final[int] = 1000
MIN_INTEGRAL: Final[int] = -1000


class DroneController:
    def __init__(self) -> None:
        self.kp_x = DEFAULT_PROP_GAIN_X
        self.ki_x = DEFAULT_INT_GAIN_X
        self.kd_x = DEFAULT_DER_GAIN_X

        self.kp_y = DEFAULT_PROP_GAIN_Y
        self.ki_y = DEFAULT_INT_GAIN_Y
        self.kd_y = DEFAULT_DER_GAIN_Y

        self.kp_z = DEFAULT_PROP_GAIN_Z
        self.ki_z = DEFAULT_INT_GAIN_Z
        self.kd_z = DEFAULT_DER_GAIN_Z

        self.target_area = INITIAL_TARGET_AREA
        self.max_area_growth = MAX_AREA_GROWTH

        self.max_velocity = MAX_VELOCITY
        self.max_acceleration = MAX_ACCELERATION

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

        # Dynamic area growth adjustment with damping
        area_error = self.target_area - area
        area_growth_rate = min(self.max_area_growth, abs(area_error) * DAMPING_FACTOR)
        self.target_area += area_growth_rate * copysign(1, area_error)

        # PID Components
        self.integral_x = clamp(self.integral_x + x_error, MAX_INTEGRAL, MIN_INTEGRAL)
        self.integral_y = clamp(self.integral_y + y_error, MAX_INTEGRAL, MIN_INTEGRAL)
        self.integral_z = clamp(self.integral_z + area_error, MAX_INTEGRAL, MIN_INTEGRAL)

        derivative_x = x_error - self.prev_x_error
        derivative_y = y_error - self.prev_y_error
        derivative_z = area_error - self.prev_z_error

        # Compute velocities
        x_velocity = (self.kp_x * x_error) + (self.ki_x * self.integral_x) + (self.kd_x * derivative_x)
        y_velocity = (self.kp_y * y_error) + (self.ki_y * self.integral_y) + (self.kd_y * derivative_y)
        z_velocity = (self.kp_z * area_error) + (self.ki_z * self.integral_z) + (self.kd_z * derivative_z)

        # Cap velocities to within a set bound
        x_velocity = clamp(x_velocity, self.max_velocity, -self.max_velocity)
        y_velocity = clamp(y_velocity, self.max_velocity, -self.max_velocity)
        z_velocity = clamp(z_velocity, self.max_velocity, -self.max_velocity)

        # Cap change in velocity to within a set bound
        x_velocity = clamp(x_velocity, self.prev_x_velocity + MAX_ACCELERATION, self.prev_x_velocity - MAX_ACCELERATION)
        y_velocity = clamp(y_velocity, self.prev_y_velocity + MAX_ACCELERATION, self.prev_y_velocity - MAX_ACCELERATION)
        z_velocity = clamp(z_velocity, self.prev_z_velocity + MAX_ACCELERATION, self.prev_z_velocity - MAX_ACCELERATION)

        self.prev_x_velocity = x_velocity
        self.prev_y_velocity = y_velocity
        self.prev_z_velocity = z_velocity

        # Update previous errors
        self.prev_x_error = x_error
        self.prev_y_error = y_error
        self.prev_z_error = area_error

        return x_velocity, y_velocity, z_velocity


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
