"""
@file server.py
@brief UNIX Socket Server
"""

import json
import os
import socket
from typing import Dict, Final, Optional, Tuple

from log import logger

DEFAULT_SOCKET_NAME: Final[str] = "DRIFT.sock"
DEFAULT_SOCKET_DIR: Final[str] = "/run/"

OUTPUT_MESSAGE_SIZE: Final[int] = 10000
MAX_SOCKET_CLIENTS: Final[int] = 20

MESSAGE_ID_FIELD_NAME: Final[str] = "id";
DETECTION_AREA_FIELD_NAME: Final[str] = "det_area";
DELTA_X_FIELD_NAME: Final[str] = "x_off";
DELTA_Y_FIELD_NAME: Final[str] = "y_off";


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
        logger.info("UnixSockerServer::accept_client(): Waiting for client connection")
        self.client_connection, client_address = self.server_socket.accept()
        logger.info("UnixSocketServer::accept_client(): Client connection established")


    def get_message_from_client(self) -> Optional[Dict[str, int]]:
        """ Reads a single message from the current client connected to the
                socket

        Returns:
            Dictionary representation of a JSON object or nothing if the client
                has disconnected from the server
        """
        data = self.client_connection.recv(OUTPUT_MESSAGE_SIZE)

        if not data:
            self.client_connection.close()
            logger.info("UnixSocketServer::get_message_from_client(): Client connection closed")

        try:
            json_data = json.loads(data.decode('utf-8'))
            logger.info(f"UnixSocketServer::get_message_from_client(): Received: {json_data}")
            return json_data
        except:
            logger.warning("UnixSocketServer::get_message_from_client(): Invalid JSON message")


    def extract_values_from_message(self) -> Tuple[int, int, int]:
        """ """
        message = self.get_message_from_client()
        if not message:
            logger.info("UnixSocketServer::extract_values_from_message(): Client disconnected, no message")
            return -1, -1, -1

        return message[DELTA_X_FIELD_NAME], message[DELTA_Y_FIELD_NAME], message[DETECTION_AREA_FIELD_NAME]


    def stop(self) -> None:
        """ Stop the server and clean up the socket file """
        if self.server_socket:
            self.server_socket.close()
            logger.info("UnixSockerServer::stop(): Server stopped")
        self._cleanup_socket()



if __name__ == "__main__":
    server = UnixSocketServer()
    try:
        server.start_server()
        server.accept_client()
    except KeyboardInterrupt:
        logger.info("main(): Server is shutting down")
    finally:
        server.stop()
