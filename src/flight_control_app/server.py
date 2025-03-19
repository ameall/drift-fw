import json
import os
import socket
from typing import Dict

DEFAULT_SOCKET_NAME = "DRIFT.sock"
DEFAULT_SOCKET_DIR = "/run/"

OUTPUT_MESSAGE_SIZE = 10000
MAX_SOCKET_CLIENTS = 20

class UnixSocketServer:
    """ UNIX Socket Server Manager

    Attributes: 
        socket_name: Filename of the socket file
        socket_path: Full path to the socket file
        server_socket: UNIX server
        client_connection: Single client connection on the socket
    """
    def __init__(self, socket_name: str = DEFAULT_SOCKET_NAME):
        """ Determines the name of the server based on if the system uses XDG

        Args:
            socket_name: Name of the actual socket file
        """
        def get_socket_path() -> str:
            """ Determines the path to the socket based on if the system uses
                    XDG or not

            Returns:
                A string representing the path to the directory to place the
                    socket
            """
            xdg_runtime_dir = os.getenv("XDG_RUNTIME_DIR")
            return f"{xdg_runtime_dir}/{self.socket_name}" if xdg_runtime_dir else f"{DEFAULT_SOCKET_DIR}/{self.socket_name}"

        self.socket_name = socket_name
        self.socket_path = get_socket_path()

    def _cleanup_socket(self) -> None:
        """ Cleans up the socket file in the system """
        if os.path.exists(self.socket_path):
            os.unlink(self.socket_path)

    def start_server(self) -> None:
        """ Creates the server and binds it to the socket file """
        self._cleanup_socket()

        self.server_socket = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
        self.server_socket.bind(self.socket_path)
        self.server_socket.listen(MAX_SOCKET_CLIENTS)

    def accept_client(self) -> None:
        """ Accepts a client connection on the UNIX socket """
        print("Waiting")
        self.client_connection, client_address = self.server_socket.accept()
        print("Connection established")

    def get_message_from_client(self) -> None | Dict:
        """ Reads a single message from the current client connected to the
                socket

        Returns:
            Dictionary representation of a JSON object or nothing if the client
                has disconnected from the server
        """
        data = self.client_connection.recv(OUTPUT_MESSAGE_SIZE)

        if not data:
            self.client_connection.close()
            print("Client disconnected")

        try:
            json_data = json.loads(data.decode('utf-8'))
            print(f"Received: {json_data}")
            return json_data
        except:
            print("Invalid JSON message")

    def handle_connections(self) -> None:
        """Handle incoming client connections."""
        while True:
            self.accept_client()

            while True:
                self.get_message_from_client()

    def stop(self) -> None:
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
