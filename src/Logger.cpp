#include "Logger.h"

#include "LogManager.h"

bool Logger::addAppender(Appender& appender) {
  for (uint8_t i = 0; i < appender_count_; ++i) {
    if (appenders_[i] == &appender) return true;
  }
  if (appender_count_ >= LOG4ESP32_MAX_APPENDERS_PER_LOGGER) return false;
  appenders_[appender_count_++] = &appender;
  return true;
}

void Logger::dispatch(const LoggingEvent& event) {
  for (uint8_t i = 0; i < appender_count_; ++i) {
    appenders_[i]->append(event);
  }
  if (additivity_ && parent_ != nullptr) parent_->dispatch(event);
}
