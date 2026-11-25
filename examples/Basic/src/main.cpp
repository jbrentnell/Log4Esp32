#include <Log4Esp32.h>

namespace {
Logger* logger = nullptr;
}

void setup() {
  Serial.begin(115200);
  LogManager::begin();
  logger = LogManager::getLogger("Basic");
  if (logger != nullptr) logger->info("Log4Esp32 started");
}

void loop() {
}
