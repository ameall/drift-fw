"""
@file logging.py
@brief Defines logging format and functions
"""

import logging

log_format = "[%(asctime)s] [%(levelname)s] - %(message)s"
date_format = "%Y-%m-%d %H-%M-%S"

logging.basicConfig(
        level=logging.DEBUG,
        format=log_format,
        datefmt=date_format
)

logger = logging.getLogger("drift-control")
