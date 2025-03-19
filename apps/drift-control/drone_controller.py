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

from pixel_displacement_simulator import CameraAppSimulator

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

# Example usage
controller = DroneController()
simulator = CameraAppSimulator(50, -25, 2500)
for _ in range(100):  # Simulating 10 frames
    x, y, z = simulator.run()
    fwd, up, right, _ = controller.compute_velocity(z,y,x)
    print("Velocity Vector:", round(fwd,ndigits=3), round(up,ndigits=3), right)
