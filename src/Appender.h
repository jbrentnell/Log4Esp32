#pragma once

#include "Formatter.h"

enum class AppenderType : uint8_t {
  Serial,
  Null,
};

// Destination-independent appender contract.
class Appender {
 public:
  explicit Appender(const char* name) : name_(name) {}
  virtual ~Appender() = default;

  const char* name() const { return name_; }
  virtual AppenderType type() const = 0;
  void setLevel(LogLevel level) { level_ = level; }
  LogLevel level() const { return level_; }
  void setEnabled(bool enabled) { enabled_ = enabled; }
  bool isEnabled() const { return enabled_; }

  // Applies the appender threshold before handing an event to the destination.
  void append(const LoggingEvent& event) {
    if (enabled_ && logLevelAllows(level_, event.level)) appendEvent(event);
  }

  virtual void setFormatter(Formatter* formatter) = 0;

 protected:
  virtual void appendEvent(const LoggingEvent& event) = 0;

  const char* name_;
  LogLevel level_ = LogLevel::Info;
  bool enabled_ = true;
};
