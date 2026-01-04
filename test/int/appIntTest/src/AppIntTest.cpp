/// Integration tests for application-level scenarios.
///
/// This test suite validates end-to-end use cases that exercise
/// multiple components working together as they would in production.

#include <logger/ILogger.hpp>
#include <logger/LoggerFactory.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

namespace sample::inttest {

/// Test suite for application logging integration.
///
/// These tests validate realistic logging scenarios that an application
/// might encounter during normal operation.
class AppLoggingIntSuite : public ::testing::Test {
protected:
  /// Get the logger instance.
  [[nodiscard]] auto logger() const -> logger::ILogger & { return *logger_; }

  /// Check if the logger is valid.
  [[nodiscard]] auto hasLogger() const -> bool { return logger_ != nullptr; }

  /// Set up a logger instance for integration testing.
  void SetUp() override { logger_ = logger::createDefaultLogger(); }

  /// Clean up logger instance after each test.
  void TearDown() override { logger_.reset(); }

private:
  std::unique_ptr<logger::ILogger> logger_;
};

/// Test suite for application startup/shutdown scenarios.
class AppLifecycleSuite : public ::testing::Test {
protected:
  /// No special setup needed.
  void SetUp() override {}

  /// No special teardown needed.
  void TearDown() override {}
};

// Test case: Scenario - Application logs startup sequence
TEST_F(AppLoggingIntSuite, ApplicationStartupSequence) {
  // Scenario: Application starts and logs initialization steps
  EXPECT_NO_THROW({
    logger().log("Application starting...");
    logger().log("Loading configuration");
    logger().log("Initializing subsystems");
    logger().log("Startup complete");
  }) << "Startup sequence should complete without errors";
}

// Test case: Scenario - Application logs during normal operation
TEST_F(AppLoggingIntSuite, NormalOperationLogging) {
  // Scenario: Application processes requests and logs activity
  ASSERT_TRUE(hasLogger()) << "Logger must be available during operation";

  EXPECT_NO_THROW({
    logger().log("Processing request #1");
    logger().log("Request #1 completed successfully");
    logger().log("Processing request #2");
    logger().log("Request #2 completed successfully");
  }) << "Normal operation logging should work correctly";
}

// Test case: Scenario - Application logs error conditions
TEST_F(AppLoggingIntSuite, ErrorConditionLogging) {
  // Scenario: Application encounters and logs error conditions
  ASSERT_TRUE(hasLogger()) << "Logger must be available for errors";

  EXPECT_NO_THROW({
    logger().log("ERROR: Failed to open configuration file");
    logger().log("WARNING: Using default configuration");
    logger().log("INFO: Retrying operation...");
    logger().log("INFO: Operation succeeded on retry");
  }) << "Error logging should work correctly";
}

// Test case: Scenario - Application logs shutdown sequence
TEST_F(AppLoggingIntSuite, ApplicationShutdownSequence) {
  // Scenario: Application shuts down gracefully with logging
  ASSERT_TRUE(hasLogger()) << "Logger must be available for shutdown";

  EXPECT_NO_THROW({
    logger().log("Shutdown initiated");
    logger().log("Closing active connections");
    logger().log("Saving state");
    logger().log("Cleanup complete");
    logger().log("Application terminated");
  }) << "Shutdown sequence should complete without errors";
}

// Test case: Scenario - High-volume logging scenario
TEST_F(AppLoggingIntSuite, HighVolumeLoggingScenario) {
  // Scenario: Application logs many messages in quick succession
  ASSERT_TRUE(hasLogger()) << "Logger must handle high volume";

  EXPECT_NO_THROW({
    for (int i = 0; i < 100; ++i) {
      logger().log("Log message " + std::to_string(i));
    }
  }) << "High-volume logging should complete without errors";
}

// Test case: Scenario - Complete application lifecycle
TEST_F(AppLifecycleSuite, CompleteApplicationLifecycle) {
  // Scenario: Full application lifecycle with logging
  auto logger = logger::createDefaultLogger();
  ASSERT_NE(logger, nullptr) << "Logger creation must succeed";

  // Startup phase
  EXPECT_NO_THROW(logger->log("=== Application Starting ==="))
      << "Startup logging should work";

  // Operation phase
  EXPECT_NO_THROW({
    logger->log("Performing work...");
    logger->log("Work completed");
  }) << "Operation logging should work";

  // Shutdown phase
  EXPECT_NO_THROW(logger->log("=== Application Shutting Down ==="))
      << "Shutdown logging should work";
}

// Test case: Scenario - Multiple components logging independently
TEST_F(AppLifecycleSuite, MultiComponentLoggingScenario) {
  // Scenario: Different components use separate logger instances
  auto loggerA = logger::createDefaultLogger();
  auto loggerB = logger::createDefaultLogger();

  ASSERT_NE(loggerA, nullptr) << "Component A logger must be valid";
  ASSERT_NE(loggerB, nullptr) << "Component B logger must be valid";

  EXPECT_NO_THROW({
    loggerA->log("Component A: Processing task");
    loggerB->log("Component B: Processing task");
    loggerA->log("Component A: Task complete");
    loggerB->log("Component B: Task complete");
  }) << "Multiple components should log independently";
}

// Value-parameterised test: Various application scenarios
class AppScenarioTest
    : public ::testing::TestWithParam<std::vector<std::string>> {
protected:
  /// Get the logger instance.
  [[nodiscard]] auto logger() const -> logger::ILogger & { return *logger_; }

  /// Set up a logger instance for parameterised testing.
  void SetUp() override { logger_ = logger::createDefaultLogger(); }

  /// Clean up logger instance after each test.
  void TearDown() override { logger_.reset(); }

private:
  std::unique_ptr<logger::ILogger> logger_;
};

TEST_P(AppScenarioTest, ScenarioMessageSequence) {
  // Scenario: Execute a sequence of log messages
  const auto &messages = GetParam();

  EXPECT_NO_THROW({
    for (const auto &message : messages) {
      logger().log(message);
    }
  }) << "Message sequence should complete without errors";
}

// Define test scenarios as message sequences
INSTANTIATE_TEST_SUITE_P(
    TypicalScenarios, AppScenarioTest,
    ::testing::Values(
        std::vector<std::string>{"Database connection opened", "Query executed",
                                 "Results retrieved",
                                 "Database connection closed"},
        std::vector<std::string>{"User login attempt", "Authentication success",
                                 "Session created"},
        std::vector<std::string>{"File upload started", "Validating file",
                                 "Processing file", "Upload complete"}));

} // namespace sample::inttest
