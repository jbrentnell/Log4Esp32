#include <unity.h>

#include "LoggingEvent.h"
#include "LogLevel.h"
#include "PatternFormatter.h"

void test_log_level_thresholds() {
  TEST_ASSERT_TRUE(logLevelAllows(LogLevel::Info, LogLevel::Info));
  TEST_ASSERT_TRUE(logLevelAllows(LogLevel::Info, LogLevel::Error));
  TEST_ASSERT_FALSE(logLevelAllows(LogLevel::Info, LogLevel::Debug));
  TEST_ASSERT_FALSE(logLevelAllows(LogLevel::Off, LogLevel::Fatal));
}

void test_event_truncates_logger_name() {
  LoggingEvent event;
  event.setLogger("PowerCart.Motor.LeftController.WithAnExcessivelyLongName");

  TEST_ASSERT_EQUAL_STRING("PowerCart.Motor.LeftC[truncated]", event.logger);
}

void test_event_truncates_task_name() {
  LoggingEvent event;
  event.setTask("VeryLongFreeRTOSTaskName");

  TEST_ASSERT_EQUAL_STRING("VeryL[truncated]", event.task);
}

void test_pattern_formatter_formats_event() {
  LoggingEvent event;
  event.level = LogLevel::Warn;
  event.elapsed_ms = 12345;
  event.setLogger("PowerCart.Motor");
  event.setTask("MotorTask");
  strncpy(event.message, "overcurrent", sizeof(event.message) - 1);

  PatternFormatter formatter("test", "%r %-5p [%t] %c - %m%n");
  char output[128] = {};
  formatter.format(event, output, sizeof(output));

  TEST_ASSERT_EQUAL_STRING("12.345 WARN  [MotorTask] PowerCart.Motor - overcurrent\n",
                           output);
}

void test_pattern_formatter_supports_literal_percent_and_unknown_conversion() {
  LoggingEvent event;
  strncpy(event.message, "ready", sizeof(event.message) - 1);

  PatternFormatter formatter("test", "100%% %z %m");
  char output[64] = {};
  formatter.format(event, output, sizeof(output));

  TEST_ASSERT_EQUAL_STRING("100% %z ready", output);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_log_level_thresholds);
  RUN_TEST(test_event_truncates_logger_name);
  RUN_TEST(test_event_truncates_task_name);
  RUN_TEST(test_pattern_formatter_formats_event);
  RUN_TEST(test_pattern_formatter_supports_literal_percent_and_unknown_conversion);
  return UNITY_END();
}
