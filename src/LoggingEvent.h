#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <string.h>

#include "Log4Esp32Config.h"
#include "LogLevel.h"

// Self-contained record copied into each asynchronous appender queue.
struct LoggingEvent {
  // Captured at the original logging call, not by an appender worker.
  LogLevel level = LogLevel::Info;
  uint32_t elapsed_ms = 0;
  time_t wall_time = 0;
  bool wall_time_valid = false;

  // Fixed-size fields keep queue memory predictable on the ESP32.
  char logger[LOG4ESP32_MAX_LOGGER_NAME_LENGTH + 1] = {};
  char task[LOG4ESP32_MAX_TASK_NAME_LENGTH + 1] = {};
  char message[LOG4ESP32_MAX_MESSAGE_LENGTH + 1] = {};

  // Copy a logger name into the event, adding the truncation marker if needed.
  void setLogger(const char* value) {
    copyTruncated(logger, sizeof(logger), value);
  }

  // Copy the originating FreeRTOS task name into the event.
  void setTask(const char* value) {
    copyTruncated(task, sizeof(task), value);
  }

 private:
  // Copy a bounded string without allocating memory.
  static void copyTruncated(char* destination, size_t capacity,
                            const char* source) {
    if (capacity == 0) {
      return;
    }

    if (source == nullptr) {
      destination[0] = '\0';
      return;
    }

    const size_t limit = capacity - 1;
    const size_t length = strnlen(source, limit + 1);
    if (length <= limit) {
      memcpy(destination, source, length + 1);
      return;
    }

    static constexpr char marker[] = "[truncated]";
    const size_t marker_length = sizeof(marker) - 1;
    const size_t prefix_length = marker_length < limit
                                     ? limit - marker_length
                                     : 0;
    memcpy(destination, source, prefix_length);
    memcpy(destination + prefix_length, marker,
           min(marker_length, limit));
    destination[limit] = '\0';
  }
};
