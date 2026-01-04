#pragma once

#include <string_view>

namespace sample::logger {

/// Abstract interface for logging messages.
///
/// Implementations must be thread-safe if used in multi-threaded contexts.
class ILogger {
public:
  ILogger() = default;
  virtual ~ILogger() = default;

  ILogger(const ILogger &) = delete;
  auto operator=(const ILogger &) -> ILogger & = delete;
  ILogger(ILogger &&) = delete;
  auto operator=(ILogger &&) -> ILogger & = delete;

  /// Log a message.
  ///
  /// ## Parameters
  /// - `message`: The message to log. Must be valid UTF-8.
  virtual void log(std::string_view message) = 0;
};

} // namespace sample::logger
