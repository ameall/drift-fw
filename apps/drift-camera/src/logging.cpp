/**
 * @file logging.cpp
 * @brief Wrapped functions for logging messages to the console
 */

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <fstream>

#include "logging.hpp"

/**
 * @brief Gets and formats the current timestamp
 *
 * @return C-string containing the formatted timestamp
 */
static const char* get_current_timestamp()
{
    static char timestamp_buffer[20];
    std::time_t current_time = std::time(nullptr);
    std::strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&current_time));
    return timestamp_buffer;
}

/**
 * @brief Converts the enum log level into a tag string
 *
 * @param level Log message level to convert
 * @return C-string containing the message level as a string
 */
static const char* log_level_to_string(log_level level)
{
    switch (level) {
    case ERROR:
        return "ERROR";
    case INFO:
        return "INFO";
    default:
        return "NONE";
    }
}

void log_message(const log_level level, const char* format, ...)
{
    char message_buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(message_buffer, sizeof(message_buffer), format, args);
    va_end(args);

    std::FILE* output = (level == ERROR) ? stderr : stdout;

    fprintf(output, "[%s] [%s] - %s\n", get_current_timestamp(), log_level_to_string(level), message_buffer);
    fflush(output);
}
