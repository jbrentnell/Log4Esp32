#pragma once

#include <stdint.h>

// Message severity, ordered from least to most severe.
enum class LogLevel : uint8_t {
  Off = 0,
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
};

// Returns true when a threshold permits a message of the given severity.
constexpr bool logLevelAllows(LogLevel threshold, LogLevel message) {
  return threshold != LogLevel::Off &&
         static_cast<uint8_t>(message) >= static_cast<uint8_t>(threshold);
}
