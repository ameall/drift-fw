from os import wait
from gpiozero import AngularServo
from log import logger
import time

SERVO_1_PIN = 12
SERVO_2_PIN = 13
SERVO_DROP_ANGLE = 60

class GPSDeploy:
    """ GPS Tracker Drop Manager

    Attributes: 
        s1: First servo
        s2: Second servo
    """
    def __init__(self):
        """ Sets up servos """

    def setup_servos(self):
        """ Connects to servos """
        logger.info("Servos Setup")
        self.s1 = AngularServo(SERVO_1_PIN)
        time.sleep(0.1)
        self.s2 = AngularServo(SERVO_2_PIN)
        time.sleep(0.1)

    def drop_payload(self):
        """ Sets the servos to a position such that the GPS payload will drop """
        logger.info("Dropping deez")
        self.s1.angle = SERVO_DROP_ANGLE
        time.sleep(0.1)
        self.s2.angle = SERVO_DROP_ANGLE
        time.sleep(2)
