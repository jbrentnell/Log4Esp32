#pragma once

#include "LoggingEvent.h"

enum class FormatterType : uint8_t {
  Pattern,
};

// Converts a structured event into text for an appender destination.
class Formatter {
 public:
  explicit Formatter(const char* name) : name_(name) {}
  virtual ~Formatter() = default;
  const char* name() const { return name_; }
  virtual FormatterType type() const = 0;
  virtual size_t format(const LoggingEvent& event, char* buffer,
                        size_t capacity) const = 0;

 protected:
  const char* name_;
};
