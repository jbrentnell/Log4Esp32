#pragma once

#include <Arduino.h>
#include <stdarg.h>
#include <freertos/task.h>
#include <time.h>

#include "Appender.h"
#include "Log4Esp32Config.h"

class LogManager;

// Named logging source with hierarchical level and appender configuration.
class Logger {
 public:
  const char* name() const { return name_; }
  void setLevel(LogLevel level) { level_ = level; }
  LogLevel level() const { return level_; }
  void setAdditivity(bool enabled) { additivity_ = enabled; }
  bool additivity() const { return additivity_; }
  bool addAppender(Appender& appender);
  bool isEnabled(LogLevel level) const { return logLevelAllows(level_, level); }

  template <typename... Args>
  void trace(const char* format, Args... args) {
    log(LogLevel::Trace, format, args...);
  }
  template <typename... Args>
  void debug(const char* format, Args... args) {
    log(LogLevel::Debug, format, args...);
  }
  template <typename... Args>
  void info(const char* format, Args... args) {
    log(LogLevel::Info, format, args...);
  }
  template <typename... Args>
  void warn(const char* format, Args... args) {
    log(LogLevel::Warn, format, args...);
  }
  template <typename... Args>
  void error(const char* format, Args... args) {
    log(LogLevel::Error, format, args...);
  }
  template <typename... Args>
  void fatal(const char* format, Args... args) {
    log(LogLevel::Fatal, format, args...);
  }

 private:
  friend class LogManager;
  explicit Logger(const char* name) : name_(name) {}

  template <typename... Args>
  void log(LogLevel level, const char* format, Args... args) {
    if (!isEnabled(level)) return;
    LoggingEvent event;
    event.level = level;
    event.elapsed_ms = millis();
    event.setLogger(name_);
    event.setTask(pcTaskGetName(nullptr));
    const time_t now = time(nullptr);
    event.wall_time = now;
    event.wall_time_valid = now >= 1577836800;  // 2020-01-01 UTC.
    const int message_length = snprintf(event.message, sizeof(event.message),
                                        format, args...);
    if (message_length >= static_cast<int>(sizeof(event.message))) {
      static constexpr char marker[] = "[truncated]";
      constexpr size_t marker_length = sizeof(marker) - 1;
      const size_t prefix_length = sizeof(event.message) - 1 > marker_length
                                       ? sizeof(event.message) - 1 - marker_length
                                       : 0;
      memcpy(event.message + prefix_length, marker, marker_length);
      event.message[sizeof(event.message) - 1] = '\0';
    }
    dispatch(event);
  }

  void dispatch(const LoggingEvent& event);

  const char* name_;
  LogLevel level_ = LogLevel::Info;
  bool additivity_ = true;
  Appender* appenders_[LOG4ESP32_MAX_APPENDERS_PER_LOGGER] = {};
  uint8_t appender_count_ = 0;
  Logger* parent_ = nullptr;
};
