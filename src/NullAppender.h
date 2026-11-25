#pragma once

#include "Appender.h"

// Appender used to discard accepted events without allocating or formatting.
class NullAppender final : public Appender {
 public:
  explicit NullAppender(const char* name) : Appender(name) {}
  AppenderType type() const override { return AppenderType::Null; }
  void setFormatter(Formatter*) override {}

 protected:
  void appendEvent(const LoggingEvent&) override {}
};
