import asyncio
import time

from log import logger

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

def main():
    simulator = CameraAppSimulator()
    while (True):
        x, y, z = simulator.run()
        logger.info(x,y,z)

if __name__ == "__main__":
    asyncio.run(main())
