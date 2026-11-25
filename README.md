# Log4Esp32

# WARNING: THIS CODE IS UNDER DEVELOPMENT AND IS NOT READY FOR USE

An Arduino logging framework for ESP32 projects.

The current implementation provides named loggers, severity filtering, pattern
formatting, asynchronous Serial output, a Null appender, and LittleFS JSON
configuration using ArduinoJson. See [DESIGN.md](DESIGN.md) for the resource
limits and configuration decisions.

## Basic usage

```cpp
#include <Log4Esp32.h>

void setup() {
  Serial.begin(115200);
  LogManager::begin();

  Logger* log = LogManager::getLogger("PowerCart.Motor");
  log->info("Motor initialized");
}
```

The default configuration file is `/log4esp32.json`. If it is unavailable,
Log4Esp32 uses a built-in INFO-level Serial configuration.
