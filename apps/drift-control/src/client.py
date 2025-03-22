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
        "det_area": 2000,
        "x_off": 0,
        "y_off": 0
}

def simulated_run():
    json_message = json.dumps(message)   
    send_message(json_message)
    time.sleep(0.5)

client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
client_socket.connect(SOCKET_PATH)

def send_message(message):
    message_json = json.dumps(message)
    client_socket.sendall(message_json.encode('utf-8'))


if __name__ == "__main__":
    while True:
        simulated_run()
    send_message({"key": "value"})
