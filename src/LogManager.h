#pragma once

#include <Arduino.h>

#include "Logger.h"
#include "NullAppender.h"
#include "PatternFormatter.h"
#include "SerialAppender.h"

// Owns all logger, appender, and formatter instances for the device lifetime.
class LogManager {
 public:
  static bool begin(const char* path = "/log4esp32.json");
  static bool loadConfig(const char* path);

  static Logger* getLogger(const char* name);
  static PatternFormatter* createPatternFormatter(const char* name,
                                                   const char* pattern);
  static SerialAppender* createSerialAppender(const char* name,
                                              Formatter* formatter = nullptr);
  static NullAppender* createNullAppender(const char* name);

  static bool addAppender(const char* logger_name, const char* appender_name);
  static bool setFormatter(const char* appender_name,
                           const char* formatter_name);
  static bool setLoggerLevel(const char* logger_name, LogLevel level);
  static bool setAppenderLevel(const char* appender_name, LogLevel level);
  static bool setAppenderEnabled(const char* appender_name, bool enabled);

 private:
  LogManager() = delete;
  static void report(const char* message);
  static Appender* findAppender(const char* name);
  static Formatter* findFormatter(const char* name);
  static Logger* findLogger(const char* name);
  static bool installDefaults();
};
