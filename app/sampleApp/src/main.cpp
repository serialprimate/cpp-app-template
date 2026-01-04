#include <logger/ILogger.hpp>
#include <logger/LoggerFactory.hpp>

#include <cstdlib>

/// Application entry point (composition root).
///
/// Wires dependencies and delegates to application logic.
///
/// ## Returns
/// `EXIT_SUCCESS` on success.
auto main() -> int {
  // Composition root: Create dependencies
  auto logger = sample::logger::createDefaultLogger();

  // Application logic
  logger->log("Application started");
  logger->log("Hello from cpp-app-template!");
  logger->log("Application finished");

  return EXIT_SUCCESS;
}
