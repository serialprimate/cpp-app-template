/// Unit tests for LoggerFactory.
///
/// This test suite validates the logger factory functions and ensures
/// the created logger instances meet their interface contracts.

#include <logger/ILogger.hpp>
#include <logger/LoggerFactory.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace sample::logger::test {

namespace {

/// Test suite for LoggerFactory.
///
/// Validates factory functions that create logger instances.
class LoggerFactoryTest : public ::testing::Test {
protected:
  /// Verify that a logger can be created successfully.
  void SetUp() override {
    // No special setup needed for these tests
  }

  /// Clean up after each test.
  void TearDown() override {
    // No special teardown needed
  }
};

} // namespace

// Test case: Factory creates a non-null logger instance
TEST_F(LoggerFactoryTest, CreateDefaultLoggerReturnsNonNull) {
  auto logger = createDefaultLogger();
  ASSERT_NE(logger, nullptr) << "Factory should return a valid logger instance";
}

// Test case: Logger can log a simple message without throwing
TEST_F(LoggerFactoryTest, CreatedLoggerCanLogMessage) {
  auto logger = createDefaultLogger();
  EXPECT_NO_THROW(logger->log("Test message"))
      << "Logger should handle basic message logging";
}

// Test case: Logger can handle empty string
TEST_F(LoggerFactoryTest, CreatedLoggerHandlesEmptyString) {
  auto logger = createDefaultLogger();
  EXPECT_NO_THROW(logger->log(""))
      << "Logger should handle empty string without throwing";
}

// Test case: Logger can handle multi-line messages
TEST_F(LoggerFactoryTest, CreatedLoggerHandlesMultiLineMessage) {
  auto logger = createDefaultLogger();
  EXPECT_NO_THROW(logger->log("Line 1\nLine 2\nLine 3"))
      << "Logger should handle multi-line messages";
}

// Test case: Logger can handle special characters
TEST_F(LoggerFactoryTest, CreatedLoggerHandlesSpecialCharacters) {
  auto logger = createDefaultLogger();
  EXPECT_NO_THROW(logger->log("Special chars: \t\r\n\\\"\'"))
      << "Logger should handle special characters";
}

// Test case: Multiple logger instances can be created independently
TEST_F(LoggerFactoryTest, CreateMultipleLoggerInstances) {
  auto logger1 = createDefaultLogger();
  auto logger2 = createDefaultLogger();

  ASSERT_NE(logger1, nullptr) << "First logger should be valid";
  ASSERT_NE(logger2, nullptr) << "Second logger should be valid";
  EXPECT_NE(logger1.get(), logger2.get())
      << "Factory should create distinct instances";
}

// Value-parameterised test: Logger handles various message lengths
class LoggerFactoryMessageLengthTest
    : public LoggerFactoryTest,
      public ::testing::WithParamInterface<std::size_t> {};

TEST_P(LoggerFactoryMessageLengthTest, HandlesVariousMessageLengths) {
  auto logger = createDefaultLogger();
  const std::size_t length = GetParam();
  const std::string message(length, 'x');

  EXPECT_NO_THROW(logger->log(message))
      << "Logger should handle message of length " << length;
}

// Test with messages of different lengths: 0, 1, 10, 100, 1000, 10000
INSTANTIATE_TEST_SUITE_P(MessageLengths, LoggerFactoryMessageLengthTest,
                         ::testing::Values(0, 1, 10, 100, 1000, 10000));

} // namespace sample::logger::test
