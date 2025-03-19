import socket
import json

SOCKET_PATH = '/run/user/1000/DRIFT.sock'

def send_message(message):
    client_socket = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
    client_socket.connect(SOCKET_PATH)

    message_json = json.dumps(message)
    client_socket.sendall(message_json.encode('utf-8'))

    client_socket.close()

if __name__ == "__main__":
    send_message({"key": "value"})
