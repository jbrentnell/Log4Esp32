# Log4Esp32 design

## Architecture

`Logger` creates a fixed-size `LoggingEvent` only when its effective level
allows the message. It sends the event to each attached appender. Each
appender applies its own level filter before accepting the event.

```text
Logger -> logger-level filter -> Appender -> appender-level filter
                                      -> queue or immediate handling
                                      -> Formatter -> destination
```

Logger names are case-sensitive and hierarchical, using `.` as the separator.
Child loggers inherit levels and appenders from their parents. Appender
propagation is enabled by default and may be disabled with `additivity: false`.
If the same appender is encountered more than once during propagation, it is
called more than once, matching Log4cxx-style behavior.

## Built-in types

- `SerialAppender`: asynchronous, with its own queue and FreeRTOS worker task;
  it writes to the global Arduino `Serial` object.
- `NullAppender`: immediately discards accepted events; it has no queue or
  worker task.
- `PatternFormatter`: supports Log4cxx-style conversion patterns.

Supported conversions:

```text
%p  level       %c  logger name       %m  message       %n  newline
%d  wall clock  %r  elapsed seconds   %t  FreeRTOS task name
```

`%d{...}` uses `strftime` syntax. `%r` has three fractional digits. Width and
alignment modifiers, `%%`, and literal handling for unknown or malformed
conversions are supported. Logger-name precision such as `%c{2}` is not.

The default pattern is:

```text
%r %-5p %c - %m%n
```

Wall-clock time uses the application’s local-time configuration. Until the
system clock is valid, `%d` falls back to elapsed time.

## Fixed resources

Defaults are compile-time configurable through an application-owned optional
`Log4Esp32UserConfig.h`, which overrides library defaults without modifying the
library. Default limits are:

- 64 events per asynchronous appender queue
- 256 bytes per event message
- 32 logger instances
- 8 appender instances
- 4 formatter instances
- 4096 bytes for the ArduinoJson configuration document
- 16 characters for a task name
- 32 characters for a logger name

When a queue is full, the oldest event is dropped. Oversized messages, logger
names, and task names are truncated with `[truncated]`.

The Serial worker defaults are priority `1`, stack size `1024` words, and task
name `Log4Esp32.Serial`.

## Configuration

The default path is `/log4esp32.json`. `LogManager::begin()` loads that path;
an explicit load call may use another path. JSON uses ArduinoJson and accepts
both `//` and `/* ... */` comments.

Named appenders and formatters are reusable. The exact initial type names are
`SerialAppender`, `NullAppender`, and `PatternFormatter`. The `root`,
`loggers`, `appenders`, and `formatters` sections are optional.

Unknown fields are reported to `Serial` and ignored. Invalid known values,
duplicate names, missing required fields, and unknown references are reported
to `Serial`; the affected object or field is ignored while the rest loads.
If the first configuration cannot be loaded, built-in defaults are used.

## Runtime API

`LogManager` owns all created objects. Factory methods can create loggers,
appenders, and formatters when no JSON configuration is used. Duplicate names
return the existing object. Resource-limit failures are reported to `Serial`
and return an error/null result. Objects remain until device restart.

Levels and enabled state can be changed through both `LogManager` and the
individual objects. Enabling an asynchronous appender restarts its worker;
disabling it immediately discards queued events.

`LogManager` has no shutdown operation. `FATAL` is a severity only and never
terminates or restarts the application.
