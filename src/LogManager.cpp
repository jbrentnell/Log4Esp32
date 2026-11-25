#include "LogManager.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ctype.h>

namespace {
Logger* loggers[LOG4ESP32_MAX_LOGGERS] = {};
Appender* appenders[LOG4ESP32_MAX_APPENDERS] = {};
Formatter* formatters[LOG4ESP32_MAX_FORMATTERS] = {};
uint8_t logger_count = 0;
uint8_t appender_count = 0;
uint8_t formatter_count = 0;
bool initialized = false;

char* duplicateName(const char* value) {
  const size_t length = strlen(value) + 1;
  char* copy = new char[length];
  if (copy != nullptr) memcpy(copy, value, length);
  return copy;
}

LogLevel parseLevel(const char* text, bool* valid) {
  *valid = true;
  if (text == nullptr) {
    *valid = false;
    return LogLevel::Info;
  }
  char value[12] = {};
  for (size_t i = 0; text[i] != '\0' && i < sizeof(value) - 1; ++i) {
    value[i] = static_cast<char>(toupper(text[i]));
  }
  if (!strcmp(value, "TRACE")) return LogLevel::Trace;
  if (!strcmp(value, "DEBUG")) return LogLevel::Debug;
  if (!strcmp(value, "INFO")) return LogLevel::Info;
  if (!strcmp(value, "WARN")) return LogLevel::Warn;
  if (!strcmp(value, "ERROR")) return LogLevel::Error;
  if (!strcmp(value, "FATAL")) return LogLevel::Fatal;
  if (!strcmp(value, "OFF")) return LogLevel::Off;
  *valid = false;
  return LogLevel::Info;
}
}

void LogManager::report(const char* message) {
  Serial.print("Log4Esp32: ");
  Serial.println(message);
}

Logger* LogManager::findLogger(const char* name) {
  for (uint8_t i = 0; i < logger_count; ++i) {
    if (!strcmp(loggers[i]->name(), name)) return loggers[i];
  }
  return nullptr;
}

Appender* LogManager::findAppender(const char* name) {
  for (uint8_t i = 0; i < appender_count; ++i) {
    if (!strcmp(appenders[i]->name(), name)) return appenders[i];
  }
  return nullptr;
}

Formatter* LogManager::findFormatter(const char* name) {
  for (uint8_t i = 0; i < formatter_count; ++i) {
    if (!strcmp(formatters[i]->name(), name)) return formatters[i];
  }
  return nullptr;
}

Logger* LogManager::getLogger(const char* name) {
  if (name == nullptr || *name == '\0') return nullptr;
  if (Logger* existing = findLogger(name)) return existing;
  if (logger_count >= LOG4ESP32_MAX_LOGGERS) {
    report("logger limit reached");
    return nullptr;
  }
  Logger* logger = new Logger(duplicateName(name));
  if (logger == nullptr) return nullptr;
  loggers[logger_count++] = logger;

  char parent_name[LOG4ESP32_MAX_LOGGER_NAME_LENGTH + 1] = {};
  strncpy(parent_name, name, sizeof(parent_name) - 1);
  char* separator = strrchr(parent_name, '.');
  if (separator != nullptr) {
    *separator = '\0';
    logger->parent_ = getLogger(parent_name);
    if (logger->parent_ != nullptr) logger->setLevel(logger->parent_->level());
  } else if (strcmp(name, "root") != 0) {
    logger->parent_ = getLogger("root");
    if (logger->parent_ != nullptr) logger->setLevel(logger->parent_->level());
  }
  return logger;
}

PatternFormatter* LogManager::createPatternFormatter(const char* name,
                                                     const char* pattern) {
  if (findFormatter(name) != nullptr) {
    Formatter* existing = findFormatter(name);
    if (existing->type() != FormatterType::Pattern) return nullptr;
    auto* pattern_formatter = static_cast<PatternFormatter*>(existing);
    pattern_formatter->setPattern(pattern);
    return pattern_formatter;
  }
  if (formatter_count >= LOG4ESP32_MAX_FORMATTERS) {
    report("formatter limit reached");
    return nullptr;
  }
  auto* formatter = new PatternFormatter(duplicateName(name), pattern);
  formatters[formatter_count++] = formatter;
  return formatter;
}

SerialAppender* LogManager::createSerialAppender(const char* name,
                                                 Formatter* formatter) {
  if (Appender* existing = findAppender(name)) {
    return existing->type() == AppenderType::Serial
               ? static_cast<SerialAppender*>(existing)
               : nullptr;
  }
  if (appender_count >= LOG4ESP32_MAX_APPENDERS) {
    report("appender limit reached");
    return nullptr;
  }
  auto* appender = new SerialAppender(duplicateName(name));
  if (formatter != nullptr) appender->setFormatter(formatter);
  if (!appender->begin()) {
    report("SerialAppender worker task could not be created");
    delete appender;
    return nullptr;
  }
  appenders[appender_count++] = appender;
  return appender;
}

NullAppender* LogManager::createNullAppender(const char* name) {
  if (Appender* existing = findAppender(name)) {
    return existing->type() == AppenderType::Null
               ? static_cast<NullAppender*>(existing)
               : nullptr;
  }
  if (appender_count >= LOG4ESP32_MAX_APPENDERS) {
    report("appender limit reached");
    return nullptr;
  }
  auto* appender = new NullAppender(duplicateName(name));
  appenders[appender_count++] = appender;
  return appender;
}

bool LogManager::addAppender(const char* logger_name, const char* appender_name) {
  Logger* logger = getLogger(logger_name);
  Appender* appender = findAppender(appender_name);
  return logger != nullptr && appender != nullptr && logger->addAppender(*appender);
}

bool LogManager::setFormatter(const char* appender_name,
                              const char* formatter_name) {
  Appender* appender = findAppender(appender_name);
  Formatter* formatter = findFormatter(formatter_name);
  if (appender == nullptr || formatter == nullptr) return false;
  appender->setFormatter(formatter);
  return true;
}

bool LogManager::setLoggerLevel(const char* logger_name, LogLevel level) {
  Logger* logger = getLogger(logger_name);
  if (logger == nullptr) return false;
  logger->setLevel(level);
  return true;
}

bool LogManager::setAppenderLevel(const char* appender_name, LogLevel level) {
  Appender* appender = findAppender(appender_name);
  if (appender == nullptr) return false;
  appender->setLevel(level);
  return true;
}

bool LogManager::setAppenderEnabled(const char* appender_name, bool enabled) {
  Appender* appender = findAppender(appender_name);
  if (appender == nullptr) return false;
  if (appender->type() == AppenderType::Serial) {
    static_cast<SerialAppender*>(appender)->setEnabled(enabled);
  }
  else appender->setEnabled(enabled);
  return true;
}

bool LogManager::installDefaults() {
  auto* formatter = createPatternFormatter("default", "%r %-5p %c - %m%n");
  auto* serial = createSerialAppender("serial", formatter);
  if (serial == nullptr) return false;
  serial->setLevel(LogLevel::Info);
  Logger* root = getLogger("root");
  if (root == nullptr) return false;
  root->addAppender(*serial);
  return true;
}

bool LogManager::begin(const char* path) {
  if (initialized) return true;
  if (!LittleFS.begin(false)) {
    report("LittleFS could not be mounted; using built-in defaults");
  }
  initialized = installDefaults();
  if (!initialized) return false;
  if (path == nullptr) return true;
  return loadConfig(path);
}

bool LogManager::loadConfig(const char* path) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    report("configuration file could not be opened; defaults remain active");
    return false;
  }
  DynamicJsonDocument document(LOG4ESP32_JSON_CAPACITY);
  DeserializationError error = deserializeJson(document, file);
  file.close();
  if (error) {
    report(error.c_str());
    return false;
  }

  // Formatters must exist before appenders refer to them.
  JsonObject formatter_configs = document["formatters"].as<JsonObject>();
  for (JsonPair item : formatter_configs) {
    JsonObject config = item.value().as<JsonObject>();
    const char* type = config["type"] | "PatternFormatter";
    if (strcmp(type, "PatternFormatter") != 0) {
      report("unknown formatter type");
      continue;
    }
    createPatternFormatter(item.key().c_str(),
                           config["pattern"] | "%r %-5p %c - %m%n");
  }

  // Appenders are created after formatters so their references can be bound.
  JsonObject appender_configs = document["appenders"].as<JsonObject>();
  for (JsonPair item : appender_configs) {
    JsonObject config = item.value().as<JsonObject>();
    const char* type = config["type"] | "SerialAppender";
    Appender* appender = nullptr;
    if (!strcmp(type, "SerialAppender")) {
      appender = createSerialAppender(item.key().c_str());
    } else if (!strcmp(type, "NullAppender")) {
      appender = createNullAppender(item.key().c_str());
    } else {
      report("unknown appender type");
      continue;
    }
    if (appender == nullptr) continue;

    bool valid = false;
    if (!config["level"].isNull()) {
      LogLevel level = parseLevel(config["level"] | "INFO", &valid);
      if (valid) appender->setLevel(level);
      else report("invalid appender level");
    }
    if (!config["enabled"].isNull()) {
      const bool enabled = config["enabled"] | true;
      if (appender->type() == AppenderType::Serial) {
        static_cast<SerialAppender*>(appender)->setEnabled(enabled);
      } else {
        appender->setEnabled(enabled);
      }
    }
    const char* formatter_name = config["formatter"] | "default";
    Formatter* formatter = findFormatter(formatter_name);
    if (formatter == nullptr) {
      report("appender formatter not found");
    } else {
      appender->setFormatter(formatter);
    }
  }

  JsonObject root = document["root"].as<JsonObject>();
  if (!root.isNull()) {
    bool valid = false;
    LogLevel level = parseLevel(root["level"] | "INFO", &valid);
    if (valid) setLoggerLevel("root", level);
    JsonArray root_appenders = root["appenders"].as<JsonArray>();
    for (JsonVariant value : root_appenders) {
      addAppender("root", value.as<const char*>());
    }
  }
  JsonObject loggers = document["loggers"].as<JsonObject>();
  for (JsonPair item : loggers) {
    JsonObject config = item.value().as<JsonObject>();
    Logger* logger = getLogger(item.key().c_str());
    if (logger == nullptr) continue;
    bool valid = false;
    if (!config["level"].isNull()) {
      LogLevel level = parseLevel(config["level"] | "INFO", &valid);
      if (valid) logger->setLevel(level);
    }
    if (!config["additivity"].isNull()) logger->setAdditivity(config["additivity"] | true);
    JsonArray logger_appenders = config["appenders"].as<JsonArray>();
    for (JsonVariant value : logger_appenders) {
      addAppender(item.key().c_str(), value.as<const char*>());
    }
  }
  return true;
}
