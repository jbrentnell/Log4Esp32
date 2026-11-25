#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "Appender.h"
#include "Log4Esp32Config.h"

// Asynchronous appender that writes formatted events to the global Serial.
class SerialAppender final : public Appender {
 public:
  explicit SerialAppender(const char* name);
  ~SerialAppender() override = default;

  AppenderType type() const override { return AppenderType::Serial; }
  bool begin();
  void setFormatter(Formatter* formatter) override { formatter_ = formatter; }
  void setEnabled(bool enabled);

 protected:
  void appendEvent(const LoggingEvent& event) override;

 private:
  static void taskEntry(void* context);
  void run();

  QueueHandle_t queue_ = nullptr;
  TaskHandle_t task_ = nullptr;
  Formatter* formatter_ = nullptr;
};
