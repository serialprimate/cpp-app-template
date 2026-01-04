#pragma once

#include <memory>

namespace sample::logger {

class ILogger;

/// Create a default console logger.
///
/// ## Returns
/// A unique pointer to an ILogger implementation. Never returns nullptr.
///
/// ## Throws
/// `std::runtime_error` if logger creation fails (rare).
[[nodiscard]] auto createDefaultLogger() -> std::unique_ptr<ILogger>;

} // namespace sample::logger
