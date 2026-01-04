# Architecture Guide: cpp-app-template

This document describes the architectural patterns and design principles used in this C++23 application template.

---

## Table of Contents

1. [Overview](#overview)
2. [Directory Structure](#directory-structure)
3. [Core Design Principles](#core-design-principles)
4. [Interface + Factory Pattern](#interface--factory-pattern)
5. [Composition Root Pattern](#composition-root-pattern)
6. [Dependency Injection](#dependency-injection)
7. [SOLID Principles](#solid-principles)
8. [Adding New Components](#adding-new-components)

---

## Overview

This template promotes modular, testable, and maintainable C++ code through:
- Clear separation between interface (public API) and implementation (private details)
- Factory pattern for object creation
- Composition root for dependency wiring
- Strict encapsulation to minimise coupling

---

## Directory Structure

```
cpp-app-template/
├── app/                    # Application entry points
│   └── sampleApp/
│       ├── CMakeLists.txt
│       └── src/
│           └── main.cpp    # Composition root
├── lib/                    # Modular libraries
│   └── logger/
│       ├── CMakeLists.txt
│       ├── include/        # Public API (interfaces + factories)
│       │   └── logger/
│       │       ├── ILogger.hpp
│       │       └── LoggerFactory.hpp
│       └── src/            # Private implementation
│           └── ConsoleLogger.cpp
└── test/                   # Tests
    ├── unit/               # Unit tests (library-level)
    │   └── loggerUnitTest/
    └── int/                # Integration tests (component-based)
        └── appIntTest/
```

---

## Dependency Management Architecture

### vcpkg Integration

This project uses vcpkg as a git submodule to manage external dependencies. This approach ensures:

- **Reproducibility**: The vcpkg commit is tracked, keeping the toolchain and ports consistent.
- **Portability**: The project does not rely on system-installed packages.
- **Hermeticity**: Dependency resolution is controlled via vcpkg manifest mode.

### Directory Layout

```
external/
└── vcpkg/              # Git submodule (Microsoft vcpkg)
vcpkg/
├── cache/              # Binary cache (gitignored, local-only)
└── triplets/           # Custom triplet overlays
    ├── arm64-linux-gnu.cmake
    └── toolchain-gnu.cmake
```

**Rationale:**
- `external/` is reserved for third-party submodules.
- `vcpkg/` at the repository root holds project-owned configuration and generated cache.

### Dependency Resolution Flow

1. CMake reads `vcpkg.json` (manifest file listing dependencies).
2. vcpkg resolves dependencies using the pinned baseline in `vcpkg-configuration.json`.
3. The binary cache is checked (`vcpkg/cache/`).
4. Missing packages are built from source using the selected triplet.
5. Built packages are cached for future builds.

### Architecture Support Status

**Currently supported:**
- ARM64 Linux with GNU toolchain (tested).

**Planned (untested):**
- x64 Linux (presets provided but not validated on x64 hardware).

**Key principle**: Public headers live in `include/`, private implementations in `src/`. Consumers only see interfaces, never concrete types.

---

## Core Design Principles

### 1. Interface-Based Design
All library functionality is exposed through abstract interfaces. Clients depend on abstractions, not concrete implementations.

**Example**: `ILogger` interface defines the contract; `ConsoleLogger` is a private implementation detail.

### 2. Factory Pattern
Factories provide controlled object creation. They:
- Encapsulate construction logic
- Return interface pointers (not concrete types)
- Hide implementation details from clients

**Example**: `createDefaultLogger()` returns `std::unique_ptr<ILogger>`, hiding `ConsoleLogger` entirely.

### 3. Composition Root
The application entry point (`main.cpp`) is the **only** place where concrete dependencies are wired together. This ensures:
- Clear dependency flow
- Easy testing (mock factories in tests)
- Single source of truth for application configuration

---

## Interface + Factory Pattern

### Implementation Example

#### Public Interface (`lib/logger/include/logger/ILogger.hpp`)
```cpp
#pragma once
#include <string_view>

namespace sample::logger {

/// Abstract interface for logging messages.
class ILogger {
public:
    virtual ~ILogger() = default;

    /// Log a message.
    virtual void log(std::string_view message) = 0;
};

} // namespace sample::logger
```

#### Factory Declaration (`lib/logger/include/logger/LoggerFactory.hpp`)
```cpp
#pragma once
#include <memory>

namespace sample::logger {

class ILogger;

/// Create a default console logger.
[[nodiscard]] auto createDefaultLogger() -> std::unique_ptr<ILogger>;

} // namespace sample::logger
```

#### Private Implementation (`lib/logger/src/ConsoleLogger.cpp`)
```cpp
#include <logger/ILogger.hpp>
#include <logger/LoggerFactory.hpp>
#include <iostream>

namespace sample::logger {

namespace {

// ConsoleLogger is PRIVATE - not visible to library consumers
class ConsoleLogger final : public ILogger {
public:
    void log(std::string_view message) override {
        std::cout << message << '\n';
    }
};

} // anonymous namespace

auto createDefaultLogger() -> std::unique_ptr<ILogger> {
    return std::make_unique<ConsoleLogger>();
}

} // namespace sample::logger
```

**Key points**:
- `ConsoleLogger` lives in an anonymous namespace → not exported
- Factory returns interface type → implementation swappable
- Consumers never include `ConsoleLogger.hpp` → true encapsulation

---

## Composition Root Pattern

The composition root is where all dependencies are **created and wired together**. In this template, it's always `app/*/src/main.cpp`.

### Example (`app/sampleApp/src/main.cpp`)
```cpp
#include <logger/ILogger.hpp>
#include <logger/LoggerFactory.hpp>
#include <cstdlib>

auto main() -> int {
    // Composition root: Create dependencies
    auto logger = sample::logger::createDefaultLogger();

    // Application logic
    logger->log("Application started");
    logger->log("Hello from cpp-app-template!");
    logger->log("Application finished");

    return EXIT_SUCCESS;
}
```

**Rules**:
1. `main.cpp` is the **only** place that calls factories
2. Libraries **never** call factories themselves (no hidden dependencies)
3. Tests use mock factories to inject test doubles

---

## Dependency Injection

Dependency Injection (DI) means passing dependencies **into** a component rather than having the component create them internally.

### Without DI (Bad)
```cpp
class Application {
    ConsoleLogger logger; // Hardcoded dependency
public:
    void run() {
        logger.log("Running"); // Can't test with different logger
    }
};
```

### With DI (Good)
```cpp
class Application {
    std::unique_ptr<ILogger> logger;
public:
    explicit Application(std::unique_ptr<ILogger> log)
        : logger{std::move(log)} {}

    void run() {
        logger->log("Running"); // Can inject mock logger in tests
    }
};
```

**Benefit**: You can now test `Application` with a mock logger that verifies log calls without touching stdout.

---

## SOLID Principles

This template enforces SOLID object-oriented design principles:

### S - Single Responsibility Principle
Each class/module has one reason to change.
- `ILogger`: Defines logging contract
- `ConsoleLogger`: Implements stdout logging
- `LoggerFactory`: Creates loggers

### O - Open/Closed Principle
Code is open for extension, closed for modification.
- Add new logger types (e.g., `FileLogger`) without changing `ILogger` or consumers
- Factory can switch implementations without breaking clients

### L - Liskov Substitution Principle
Derived types are substitutable for base types.
- Any `ILogger` implementation works wherever `ILogger` is expected
- Enforced by interface contract

### I - Interface Segregation Principle
Clients depend only on interfaces they use.
- `ILogger` has one method: `log()`
- No "fat interfaces" with unused methods

### D - Dependency Inversion Principle
Depend on abstractions, not concretions.
- `sampleApp` depends on `ILogger`, not `ConsoleLogger`
- High-level modules don't depend on low-level implementation details

---

## Adding New Components

### Adding a New Library

1. **Create directory structure:**
   ```bash
   mkdir -p lib/mylib/include/mylib
   mkdir -p lib/mylib/src
   ```

2. **Define public interface** (`lib/mylib/include/mylib/IMyLib.hpp`):
   ```cpp
   #pragma once

   namespace sample::mylib {

   class IMyLib {
   public:
       virtual ~IMyLib() = default;
       virtual void doSomething() = 0;
   };

   } // namespace sample::mylib
   ```

3. **Define factory** (`lib/mylib/include/mylib/MyLibFactory.hpp`):
   ```cpp
   #pragma once
   #include <memory>

   namespace sample::mylib {

   class IMyLib;

   [[nodiscard]] auto createMyLib() -> std::unique_ptr<IMyLib>;

   } // namespace sample::mylib
   ```

4. **Implement privately** (`lib/mylib/src/MyLibImpl.cpp`):
   ```cpp
   #include <mylib/IMyLib.hpp>
   #include <mylib/MyLibFactory.hpp>

   namespace sample::mylib {

   namespace {

   class MyLibImpl final : public IMyLib {
   public:
       void doSomething() override {
           // Implementation here
       }
   };

   } // anonymous namespace

   auto createMyLib() -> std::unique_ptr<IMyLib> {
       return std::make_unique<MyLibImpl>();
   }

   } // namespace sample::mylib
   ```

5. **Create CMakeLists.txt** (follow `lib/logger/CMakeLists.txt` pattern)

6. **Wire in root CMakeLists.txt:**
   ```cmake
   add_subdirectory(lib/mylib)
   ```

7. **Use in app:**
   ```cpp
   // In app/sampleApp/src/main.cpp
   #include <mylib/MyLibFactory.hpp>

   auto main() -> int {
       auto myLib = sample::mylib::createMyLib();
       myLib->doSomething();
       return 0;
   }
   ```

### Adding Tests

#### Unit Test Conventions

Unit tests validate individual library components in isolation.

**Naming and structure:**
- **Executable target:** `<lib-name>UnitTest` (e.g., `loggerUnitTest`)
- **Directory:** `test/unit/<target-name>/` (e.g., `test/unit/loggerUnitTest/`)
- **Test files:** PascalCase matching class under test (e.g., `LoggerFactoryTest.cpp`, `ConsoleLoggerTest.cpp`)
- **Test suites:** Named after the class being tested (e.g., `class LoggerFactoryTest : public ::testing::Test`)
- **Test cases:** Test individual methods or behaviours (e.g., `TEST_F(LoggerFactoryTest, CreateDefaultLoggerReturnsNonNull)`)

**Example structure:**
```
test/unit/loggerUnitTest/
├── CMakeLists.txt          # Defines loggerUnitTest target
└── src/
    ├── LoggerFactoryTest.cpp
    └── ConsoleLoggerTest.cpp (if testing concrete class)
```

**Example test file** (`LoggerFactoryTest.cpp`):
```cpp
#include <logger/ILogger.hpp>
#include <logger/LoggerFactory.hpp>
#include <gtest/gtest.h>

namespace sample::logger::test {

/// Test suite for LoggerFactory
class LoggerFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here
    }
};

// Test case: Factory creates valid logger
TEST_F(LoggerFactoryTest, CreateDefaultLoggerReturnsNonNull) {
    auto logger = createDefaultLogger();
    ASSERT_NE(logger, nullptr);
}

// Value-parameterised test for multiple scenarios
class LoggerFactoryMessageLengthTest
    : public LoggerFactoryTest,
      public ::testing::WithParamInterface<std::size_t> {};

TEST_P(LoggerFactoryMessageLengthTest, HandlesVariousMessageLengths) {
    auto logger = createDefaultLogger();
    const std::size_t length = GetParam();
    const std::string message(length, 'x');
    EXPECT_NO_THROW(logger->log(message));
}

INSTANTIATE_TEST_SUITE_P(MessageLengths,
                         LoggerFactoryMessageLengthTest,
                         ::testing::Values(0, 1, 10, 100, 1000));

} // namespace sample::logger::test
```

**CMakeLists.txt pattern:**
```cmake
cmake_minimum_required(VERSION 3.28.3)

set(TARGET_NAME loggerUnitTest)

project(
    ${TARGET_NAME}
    VERSION 0.1.0
    DESCRIPTION "Unit tests for logger library."
    LANGUAGES CXX
)

add_executable(${TARGET_NAME})

find_package(GTest CONFIG REQUIRED)

target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        logger                # Library under test
        GTest::gtest
        GTest::gtest_main
)

target_sources(
    ${TARGET_NAME}
    PRIVATE
        src/LoggerFactoryTest.cpp
)

target_compile_features(
    ${TARGET_NAME}
    PRIVATE
        cxx_std_23
)

target_compile_options(
    ${TARGET_NAME}
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra -Wpedantic -Werror>
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
```

**Key principles:**
- Link only the library-under-test and its public dependencies
- Test through public interfaces (factories), not concrete types
- Use value-parameterised tests (`TEST_P`) for testing methods with multiple input scenarios
- One test suite per class/component being tested

#### Integration Test Conventions

Integration tests validate end-to-end scenarios involving multiple components.

**Naming and structure:**
- **Executable target:** `<component-name>IntTest` (e.g., `appIntTest`, `databaseIntTest`)
- **Directory:** `test/int/<target-name>/` (e.g., `test/int/appIntTest/`)
- **Test files:** PascalCase matching component name integration subject (e.g., `AppIntTest.cpp`)
- **Test suites:** Descriptive name ending in `Suite`, using `Int` abbreviation (e.g., `class AppLoggingIntSuite : public ::testing::Test`)
- **Test cases:** Represent scenarios, not methods (e.g., `TEST_F(AppLoggingIntSuite, ApplicationStartupSequence)`)

**Example structure:**
```
test/int/appIntTest/
├── CMakeLists.txt          # Defines appIntTest target
└── src/
    └── AppIntTest.cpp
```

**Example test file** (`AppIntTest.cpp`):
```cpp
#include <logger/ILogger.hpp>
#include <logger/LoggerFactory.hpp>
#include <gtest/gtest.h>

namespace sample::inttest {

/// Test suite for application logging scenarios
class AppLoggingIntSuite : public ::testing::Test {
protected:
    std::unique_ptr<logger::ILogger> logger_;

    void SetUp() override {
        logger_ = logger::createDefaultLogger();
    }
};

// Test case: Complete application startup scenario
TEST_F(AppLoggingIntSuite, ApplicationStartupSequence) {
    ASSERT_NE(logger_, nullptr);

    EXPECT_NO_THROW({
        logger_->log("Application starting...");
        logger_->log("Loading configuration");
        logger_->log("Initializing subsystems");
        logger_->log("Application ready");
    });
}

// Test case: High-volume logging scenario
TEST_F(AppLoggingIntSuite, HighVolumeLoggingScenario) {
    });
}

} // namespace sample::inttest
```

**CMakeLists.txt pattern:**
```cmake
cmake_minimum_required(VERSION 3.28.3)

set(TARGET_NAME appIntTest)

project(
    ${TARGET_NAME}
    VERSION 0.1.0
    DESCRIPTION "Integration tests for application components."
    LANGUAGES CXX
)

add_executable(${TARGET_NAME})

find_package(GTest CONFIG REQUIRED)

target_link_libraries(
    ${TARGET_NAME}
    PRIVATE
        logger                # Can link multiple libraries
        # otherLib
        GTest::gtest
        GTest::gtest_main
)

target_sources(
    ${TARGET_NAME}
    PRIVATE
        src/AppIntTest.cpp
)

target_compile_features(
    ${TARGET_NAME}
    PRIVATE
        cxx_std_23
)

target_compile_options(
    ${TARGET_NAME}
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra -Wpedantic -Werror>
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
```

**Key principles:**
- May link multiple internal libraries (unlike unit tests)
- Test suites group related scenarios (e.g., all logging scenarios, all database scenarios)
- Test cases represent realistic use-cases from end-to-end
- Focus on component interactions, not individual class methods

---

## Best Practices

1. **Never expose concrete types in public headers**
   - Keep implementations in `src/`, not `include/`
   - Use forward declarations in factory headers

2. **Use factories for all object creation**
   - Never `new` concrete types outside their translation unit
   - Return smart pointers (`std::unique_ptr`) by default

3. **Test through interfaces**
   - Unit tests should use factories, not construct concrete types directly
   - Create mock implementations for test doubles

4. **Keep main.cpp minimal**
   - Wire dependencies → delegate to application logic
   - No business logic in the composition root

5. **Follow SOLID principles**
   - Small, focused interfaces
   - Depend on abstractions
   - Inject dependencies

---

## Further Reading

- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Dependency Injection in C++](https://www.youtube.com/watch?v=VWynp1Fgrmg)
- [Interface Segregation Principle](https://en.wikipedia.org/wiki/Interface_segregation_principle)

---

**Template Version**: 2.0.0
**Last Updated**: 2026-01-01
