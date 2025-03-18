import socket
import os
import json

DEFAULT_SOCKET_NAME = "DRIFT.sock"
DEFAULT_SOCKET_DIR = "/run/"

OUTPUT_MESSAGE_SIZE = 10000
MAX_SOCKET_CLIENTS = 20

class UnixSocketServer:
    def __init__(self, socket_name: str = DEFAULT_SOCKET_NAME):
        self.socket_name = socket_name
        self.socket_path = self.get_socket_path()

    def get_socket_path(self) -> str:
        xdg_runtime_dir = os.getenv("XDG_RUNTIME_DIR")
        return f"{xdg_runtime_dir}/{self.socket_name}" if xdg_runtime_dir else f"{DEFAULT_SOCKET_DIR}/{self.socket_name}"

    def _cleanup_socket(self) -> None:
        if os.path.exists(self.socket_path):
            os.unlink(self.socket_path)

    def start_server(self) -> None:
        self._cleanup_socket()

        self.server_socket = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
        self.server_socket.bind(self.socket_path)
        self.server_socket.listen(MAX_SOCKET_CLIENTS)

        self.handle_connections()

    def handle_connections(self):
        """Handle incoming client connections."""
        while True:
            # Accept a connection
            client_socket, client_address = self.server_socket.accept()
            print(f"Connection established with {client_address}")

            try:
                while True:
                    # Receive data from the client
                    data = client_socket.recv(OUTPUT_MESSAGE_SIZE)
                    if not data:
                        break  # No more data from the client

                    json_data = json.loads(data.decode('utf-8'))
                    print(f"Received: {data.decode('utf-8')}")

            finally:
                # Close the client socket
                client_socket.close()
                print(f"Connection with {client_address} closed.")

    def stop(self):
        """Stop the server and clean up the socket file."""
        if self.server_socket:
            self.server_socket.close()
            print("Server stopped.")
        self._cleanup_socket()


if __name__ == "__main__":
    server = UnixSocketServer()
    try:
        server.start_server()
    except KeyboardInterrupt:
        print("\nServer is shutting down...")
    finally:
        server.stop()
