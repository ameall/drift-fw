"""
This class implement as PID controller to determine the appropriate velocity 
for the drone based on the position of the car in the frame.

For this to work, we'll need to fine-tune all of the init parameters.

The compute_velocity method is what we will call inside the while-loop of the
flight_controller application.

In this setup, x is motion left/right
y is motion up/down
z is motion forward/backwards.

"""


class DroneController:
    def __init__(self, kp_x=0.01, ki_x=0, kd_x=0.005,
                 kp_y=0.01, ki_y=0, kd_y=0.005,
                 kp_z=0.02, ki_z=0, kd_z=0.01,
                 initial_target_area=20000, max_area_growth=2000):
        self.kp_x, self.ki_x, self.kd_x = kp_x, ki_x, kd_x
        self.kp_y, self.ki_y, self.kd_y = kp_y, ki_y, kd_y
        self.kp_z, self.ki_z, self.kd_z = kp_z, ki_z, kd_z

        self.target_area = initial_target_area
        self.max_area_growth = max_area_growth  # max speed to move forward at

        self.prev_x_error = 0
        self.prev_y_error = 0
        self.prev_z_error = 0

        self.integral_x = 0
        self.integral_y = 0
        self.integral_z = 0

    def compute_velocity(self, area, x, y):
        # We should start to slow down as we get closer to the car in order to avoid over-shooting.
        # to do this, we will set a maximum value for how fast we can travel forwards, as well as
        # dynamically slow down the approach as the distance gets smaller.
        distance_error = self.target_area - area  
        area_growth_rate = min(self.max_area_growth, abs(distance_error) * 0.05)

        if distance_error > 0:
            self.target_area += area_growth_rate  # Get closer
        else:
            self.target_area -= area_growth_rate  # Stay at a reasonable distance

        # Errors
        x_error = -x  
        y_error = -y  
        z_error = self.target_area - area  

        # PID Components
        self.integral_x += x_error
        self.integral_y += y_error
        self.integral_z += z_error

        derivative_x = x_error - self.prev_x_error
        derivative_y = y_error - self.prev_y_error
        derivative_z = z_error - self.prev_z_error

        # Compute velocities
        vel_x = (self.kp_x * x_error) + (self.ki_x * self.integral_x) + (self.kd_x * derivative_x)
        vel_y = (self.kp_y * y_error) + (self.ki_y * self.integral_y) + (self.kd_y * derivative_y)
        vel_z = (self.kp_z * z_error) + (self.ki_z * self.integral_z) + (self.kd_z * derivative_z)

        # Update previous errors
        self.prev_x_error = x_error
        self.prev_y_error = y_error
        self.prev_z_error = z_error

        return vel_x, vel_y, vel_z

# Example usage
controller = DroneController()
for _ in range(10):  # Simulating 10 frames
    velocity = controller.compute_velocity(area=40000, x=50, y=-30)
    print("Velocity Vector:", velocity)
