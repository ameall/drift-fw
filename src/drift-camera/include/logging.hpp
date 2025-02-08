/**
 * @file logging.hpp
 * @brief Wrapped functions for logging messages to the console
 */

/**
 * @brief Log message levels based on https://wiki.archlinux.org/title/Systemd/Journal
 */
enum log_level {
    ERROR = 3,
    INFO = 6
};

/**
 * @brief Logs a message to the console (stdout/stderr depending on the message
 *      level)
 *
 * @param level What level to log the message at
 * @param format The message to log
 */
void log_message(log_level level, const char* format, ...);
