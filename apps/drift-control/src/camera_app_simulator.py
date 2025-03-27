"""
@file client.py
@brief UNIX Socket Client
"""

import json
import socket
import time
from typing import Final

SOCKET_PATH: Final[str] = '/run/user/1000/DRIFT.sock'

message = {
        "id": 1,
        "det_area": 1800,
        "x_off": -200,
        "y_off": -300
}

counter = 20
def simulated_run():
    global counter
    counter += 1
    send_message(message)
    time.sleep(0.2)
    if counter == 10 or counter == 30:
        message["x_off"] = -message["x_off"]
    if counter == 40:
        message["y_off"] = -message["y_off"]
        counter = 0

def send_message(message):
    message_json = json.dumps(message)
    print(message_json)
    client_socket.sendall(message_json.encode('utf-8'))


if __name__ == "__main__":
    while True:
        try:
            client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
            client_socket.connect(SOCKET_PATH)
            while True:
                simulated_run()
        except FileNotFoundError:
            print("Server not available yet, retrying...")
        time.sleep(1)
