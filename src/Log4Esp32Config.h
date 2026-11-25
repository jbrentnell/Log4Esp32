#pragma once

// Compile-time defaults for the fixed-resource embedded logging system.
// Applications may provide Log4Esp32UserConfig.h in their include path to
// override these defaults without modifying the library.
#if __has_include("Log4Esp32UserConfig.h")
#include "Log4Esp32UserConfig.h"
#endif

#ifndef LOG4ESP32_QUEUE_CAPACITY
// Number of events held by each asynchronous appender.
#define LOG4ESP32_QUEUE_CAPACITY 64
#endif

#ifndef LOG4ESP32_MAX_MESSAGE_LENGTH
// Maximum formatted message length, excluding the terminating null byte.
#define LOG4ESP32_MAX_MESSAGE_LENGTH 256
#endif

#ifndef LOG4ESP32_MAX_FORMATTED_LENGTH
// Maximum output length produced by a formatter, excluding the null byte.
#define LOG4ESP32_MAX_FORMATTED_LENGTH 512
#endif

#ifndef LOG4ESP32_MAX_APPENDERS_PER_LOGGER
// Maximum directly attached appenders on one logger.
#define LOG4ESP32_MAX_APPENDERS_PER_LOGGER 8
#endif

#ifndef LOG4ESP32_MAX_LOGGERS
// Maximum number of logger objects owned by LogManager.
#define LOG4ESP32_MAX_LOGGERS 32
#endif

#ifndef LOG4ESP32_MAX_APPENDERS
// Maximum number of appender objects owned by LogManager.
#define LOG4ESP32_MAX_APPENDERS 8
#endif

#ifndef LOG4ESP32_MAX_FORMATTERS
// Maximum number of formatter objects owned by LogManager.
#define LOG4ESP32_MAX_FORMATTERS 4
#endif

#ifndef LOG4ESP32_JSON_CAPACITY
// Fixed ArduinoJson document capacity used while loading configuration.
#define LOG4ESP32_JSON_CAPACITY 4096
#endif

#ifndef LOG4ESP32_MAX_TASK_NAME_LENGTH
// Maximum task-name characters copied into a LoggingEvent.
#define LOG4ESP32_MAX_TASK_NAME_LENGTH 16
#endif

#ifndef LOG4ESP32_MAX_LOGGER_NAME_LENGTH
// Maximum logger-name characters copied into a LoggingEvent.
#define LOG4ESP32_MAX_LOGGER_NAME_LENGTH 32
#endif

#ifndef LOG4ESP32_SERIAL_TASK_PRIORITY
// Low-priority worker task used by SerialAppender.
#define LOG4ESP32_SERIAL_TASK_PRIORITY 1
#endif

#ifndef LOG4ESP32_SERIAL_TASK_STACK_SIZE
// FreeRTOS stack size in words for the SerialAppender worker.
#define LOG4ESP32_SERIAL_TASK_STACK_SIZE 1024
#endif

#ifndef LOG4ESP32_SERIAL_TASK_NAME
// Name shown by the FreeRTOS task list and diagnostics.
#define LOG4ESP32_SERIAL_TASK_NAME "Log4Esp32.Serial"
#endif
