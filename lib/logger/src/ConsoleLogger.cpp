#include <logger/ILogger.hpp>

#include <iostream>
#include <memory>
#include <string_view>

namespace sample::logger {

namespace {

/// Console logger implementation (private).
///
/// Writes log messages to stdout.
class ConsoleLogger final : public ILogger {
public:
  void log(std::string_view message) override { std::cout << message << '\n'; }
};

} // anonymous namespace

auto createDefaultLogger() -> std::unique_ptr<ILogger> {
  return std::make_unique<ConsoleLogger>();
}

} // namespace sample::logger
