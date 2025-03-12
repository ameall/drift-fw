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
    def __init__(self, kp_xy=0.005, ki_xy=0, kd_xy=0.005,
                 kp_z=0.0025, ki_z=0, kd_z=0.0025,
                 initial_target_area=20000, max_area_growth=100):
        self.kp_xy, self.ki_xy, self.kd_xy = kp_xy, ki_xy, kd_xy
        self.kp_z, self.ki_z, self.kd_z = kp_z, ki_z, kd_z

        self.target_area = initial_target_area
        self.max_area_growth = max_area_growth

        self.prev_xy_error = [0, 0]
        self.prev_z_error = 0

        self.integral_xy = [0, 0]
        self.integral_z = 0

        self.max_velocity = 5.0

    def compute_velocity(self, area, x, y):
        # Dynamic area growth adjustment with damping
        distance_error = self.target_area - area
        area_growth_rate = min(self.max_area_growth, abs(distance_error) * 0.02)
        self.target_area += area_growth_rate * (1 if distance_error > 0 else -1)

        # Errors
        xy_error = [-x, -y]
        z_error = self.target_area - area

        # PID Components
        for i in range(2):
            self.integral_xy[i] = max(min(self.integral_xy[i] + xy_error[i], 1000), -1000)

        self.integral_z = max(min(self.integral_z + z_error, 1000), -1000)

        derivative_xy = [xy_error[i] - self.prev_xy_error[i] for i in range(2)]
        derivative_z = z_error - self.prev_z_error

        # Compute velocities
        vel_x = (self.kp_xy * xy_error[0]) + (self.ki_xy * self.integral_xy[0]) + (self.kd_xy * derivative_xy[0])
        vel_y = (self.kp_xy * xy_error[1]) + (self.ki_xy * self.integral_xy[1]) + (self.kd_xy * derivative_xy[1])
        vel_z = (self.kp_z * z_error) + (self.ki_z * self.integral_z) + (self.kd_z * derivative_z)

        # Cap velocities to avoid unstable behavior
        vel_x = max(min(vel_x, self.max_velocity), -self.max_velocity)
        vel_y = max(min(vel_y, self.max_velocity), -self.max_velocity)
        vel_z = max(min(vel_z, self.max_velocity), -self.max_velocity)

        # Update previous errors
        self.prev_xy_error = xy_error
        self.prev_z_error = z_error

        return vel_z, vel_x, vel_y, 0

# Example usage
controller = DroneController()
simulator = CameraAppSimulator(50, -25, 2500)
for _ in range(100):  # Simulating 10 frames
    x, y, z = simulator.run()
    fwd, up, right, _ = controller.compute_velocity(z,y,x)
    print("Velocity Vector:", round(fwd,ndigits=3), round(up,ndigits=3), right)
