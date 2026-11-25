#include "SerialAppender.h"

#include <Arduino.h>

SerialAppender::SerialAppender(const char* name) : Appender(name) {}

bool SerialAppender::begin() {
  if (queue_ != nullptr) return task_ != nullptr;
  queue_ = xQueueCreate(LOG4ESP32_QUEUE_CAPACITY, sizeof(LoggingEvent));
  if (queue_ == nullptr) return false;
  const BaseType_t result = xTaskCreate(
      taskEntry, LOG4ESP32_SERIAL_TASK_NAME, LOG4ESP32_SERIAL_TASK_STACK_SIZE,
      this, LOG4ESP32_SERIAL_TASK_PRIORITY, &task_);
  if (result != pdPASS) {
    vQueueDelete(queue_);
    queue_ = nullptr;
    task_ = nullptr;
    return false;
  }
  return true;
}

void SerialAppender::setEnabled(bool enabled) {
  Appender::setEnabled(enabled);
  if (!enabled && queue_ != nullptr) {
    LoggingEvent discarded;
    while (xQueueReceive(queue_, &discarded, 0) == pdTRUE) {
    }
  }
}

void SerialAppender::appendEvent(const LoggingEvent& event) {
  if (queue_ == nullptr) return;
  if (xQueueSend(queue_, &event, 0) == pdTRUE) return;

  // Preserve the newest diagnostic event when the queue is full.
  LoggingEvent discarded;
  xQueueReceive(queue_, &discarded, 0);
  xQueueSend(queue_, &event, 0);
}

void SerialAppender::taskEntry(void* context) {
  static_cast<SerialAppender*>(context)->run();
}

void SerialAppender::run() {
  LoggingEvent event;
  char formatted[LOG4ESP32_MAX_FORMATTED_LENGTH + 1];
  for (;;) {
    if (xQueueReceive(queue_, &event, portMAX_DELAY) != pdTRUE) continue;
    if (!isEnabled() || formatter_ == nullptr) continue;
    formatter_->format(event, formatted, sizeof(formatted));
    Serial.print(formatted);
  }
}
