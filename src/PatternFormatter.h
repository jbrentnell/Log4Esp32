#pragma once

#include "Formatter.h"

// Log4cxx-style formatter supporting the conversion subset in DESIGN.md.
class PatternFormatter final : public Formatter {
 public:
  PatternFormatter(const char* name, const char* pattern);

  FormatterType type() const override { return FormatterType::Pattern; }
  void setPattern(const char* pattern);

  size_t format(const LoggingEvent& event, char* buffer,
                size_t capacity) const override;

  const char* pattern() const { return pattern_; }

 private:
  char pattern_[LOG4ESP32_MAX_MESSAGE_LENGTH + 1] = {};
};
