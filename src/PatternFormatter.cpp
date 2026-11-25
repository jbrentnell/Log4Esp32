#include "PatternFormatter.h"

#include <stdio.h>
#include <time.h>

namespace {

void appendText(char* out, size_t capacity, size_t* used, const char* text,
                size_t length) {
  if (*used >= capacity || text == nullptr) return;
  const size_t available = capacity - *used - 1;
  const size_t count = length < available ? length : available;
  memcpy(out + *used, text, count);
  *used += count;
  out[*used] = '\0';
}

void appendChar(char* out, size_t capacity, size_t* used, char value) {
  appendText(out, capacity, used, &value, 1);
}

void appendField(char* out, size_t capacity, size_t* used, const char* value,
                 int width, bool left) {
  if (value == nullptr) value = "";
  const size_t length = strlen(value);
  const size_t padding = width > static_cast<int>(length)
                             ? static_cast<size_t>(width) - length
                             : 0;
  if (!left) {
    for (size_t i = 0; i < padding; ++i) appendChar(out, capacity, used, ' ');
  }
  appendText(out, capacity, used, value, length);
  if (left) {
    for (size_t i = 0; i < padding; ++i) appendChar(out, capacity, used, ' ');
  }
}

void appendNumber(char* out, size_t capacity, size_t* used, uint32_t value,
                  int width, bool left) {
  char number[24];
  snprintf(number, sizeof(number), "%lu", static_cast<unsigned long>(value));
  appendField(out, capacity, used, number, width, left);
}

void appendElapsed(char* out, size_t capacity, size_t* used, uint32_t elapsed,
                   int width, bool left) {
  char number[24];
  snprintf(number, sizeof(number), "%.3f", elapsed / 1000.0f);
  appendField(out, capacity, used, number, width, left);
}

}  // namespace

PatternFormatter::PatternFormatter(const char* name, const char* pattern)
    : Formatter(name) {
  setPattern(pattern);
}

void PatternFormatter::setPattern(const char* pattern) {
  if (pattern == nullptr) pattern = "%r %-5p %c - %m%n";
  strncpy(pattern_, pattern, sizeof(pattern_) - 1);
  pattern_[sizeof(pattern_) - 1] = '\0';
}

size_t PatternFormatter::format(const LoggingEvent& event, char* buffer,
                                size_t capacity) const {
  if (buffer == nullptr || capacity == 0) return 0;
  buffer[0] = '\0';
  size_t used = 0;

  for (size_t i = 0; pattern_[i] != '\0' && used + 1 < capacity; ++i) {
    if (pattern_[i] != '%') {
      appendChar(buffer, capacity, &used, pattern_[i]);
      continue;
    }

    const size_t start = i;
    ++i;
    if (pattern_[i] == '\0') {
      appendText(buffer, capacity, &used, pattern_ + start, 1);
      break;
    }
    if (pattern_[i] == '%') {
      appendChar(buffer, capacity, &used, '%');
      continue;
    }

    bool left = false;
    if (pattern_[i] == '-') {
      left = true;
      ++i;
    }
    int width = 0;
    while (pattern_[i] >= '0' && pattern_[i] <= '9') {
      width = width * 10 + (pattern_[i] - '0');
      ++i;
    }

    const char conversion = pattern_[i];
    if (conversion == 'd' && pattern_[i + 1] == '{') {
      const char* date_start = pattern_ + i + 2;
      const char* date_end = strchr(date_start, '}');
      if (date_end == nullptr) {
        appendText(buffer, capacity, &used, pattern_ + start,
                   strlen(pattern_ + start));
        break;
      }
      char date_pattern[64];
      const size_t date_length = static_cast<size_t>(date_end - date_start);
      const size_t copy_length = date_length < sizeof(date_pattern) - 1
                                     ? date_length
                                     : sizeof(date_pattern) - 1;
      memcpy(date_pattern, date_start, copy_length);
      date_pattern[copy_length] = '\0';
      char date_output[96] = {};
      if (event.wall_time_valid) {
        struct tm local_tm;
        localtime_r(&event.wall_time, &local_tm);
        strftime(date_output, sizeof(date_output), date_pattern, &local_tm);
      } else {
        snprintf(date_output, sizeof(date_output), "%.3f",
                 event.elapsed_ms / 1000.0f);
      }
      appendField(buffer, capacity, &used, date_output, width, left);
      i = static_cast<size_t>(date_end - pattern_);
      continue;
    }

    char value[128] = {};
    switch (conversion) {
      case 'p':
        switch (event.level) {
          case LogLevel::Trace: strncpy(value, "TRACE", sizeof(value) - 1); break;
          case LogLevel::Debug: strncpy(value, "DEBUG", sizeof(value) - 1); break;
          case LogLevel::Info: strncpy(value, "INFO", sizeof(value) - 1); break;
          case LogLevel::Warn: strncpy(value, "WARN", sizeof(value) - 1); break;
          case LogLevel::Error: strncpy(value, "ERROR", sizeof(value) - 1); break;
          case LogLevel::Fatal: strncpy(value, "FATAL", sizeof(value) - 1); break;
          case LogLevel::Off: strncpy(value, "OFF", sizeof(value) - 1); break;
        }
        appendField(buffer, capacity, &used, value, width, left);
        break;
      case 'c': appendField(buffer, capacity, &used, event.logger, width, left); break;
      case 'm': appendField(buffer, capacity, &used, event.message, width, left); break;
      case 't': appendField(buffer, capacity, &used, event.task, width, left); break;
      case 'r': appendElapsed(buffer, capacity, &used, event.elapsed_ms, width, left); break;
      case 'n': appendChar(buffer, capacity, &used, '\n'); break;
      case 'd': {
        if (!event.wall_time_valid) {
          snprintf(value, sizeof(value), "%.3f", event.elapsed_ms / 1000.0f);
        } else {
          struct tm local_tm;
          localtime_r(&event.wall_time, &local_tm);
          strftime(value, sizeof(value), "%Y-%m-%d %H:%M:%S", &local_tm);
        }
        appendField(buffer, capacity, &used, value, width, left);
        break;
      }
      default:
        appendText(buffer, capacity, &used, pattern_ + start, i - start + 1);
        break;
    }
  }
  return used;
}
