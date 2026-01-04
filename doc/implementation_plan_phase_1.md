# Implementation Plan: Phase 1 MVP ("Golden Path" Template)

> **⚠️ STATUS: COMPLETED** - This implementation plan has been fully executed. This document serves as a historical record of the Phase 1 development work completed on 2026-01-02. The codebase now reflects the modular structure described herein.

| **Project** | cpp-app-template |
| :--- | :--- |
| **Phase** | Phase 1 (MVP) |
| **PRD Version** | 2.0.0 (Draft) |
| **Created** | 2025-12-31 |
| **Completed** | 2026-01-02 |
| **Status** | COMPLETED - Historical Record |

---

## 1. Technical Overview

Phase 1 transforms the existing `cpp-app-template` from a single-application
structure (`source/myapp/`) into a modular, production-ready template that
supports the full application lifecycle: apps, libraries, unit tests, and
integration tests. The key architectural change is adopting the PRD's proposed
directory layout while preserving the existing build system discipline (vcpkg
manifest mode, target-based CMake, hermetic find behaviour).

**High-level changes:**
1. **Directory restructure:** Migrate from `source/` to the PRD's modular
   layout (`app/`, `lib/`, `test/unit/`, `test/int/`).
2. **Testing infrastructure:** Add GoogleTest via vcpkg and create sample unit
   and integration tests.
3. **DevContainer bootstrap:** Enhance the DevContainer to bootstrap vcpkg
   automatically on container creation.
4. **Documentation updates:** Update README and in-tree documentation to
   reflect the new structure and golden path workflow.

**What we preserve:**
- Existing CMake presets (debug, debug-clang-tidy, release)
- Existing vcpkg manifest mode with pinned baseline
- Existing build hygiene (out-of-source enforcement, sanitiser guards)
- Existing quality tooling (.clang-format, .clang-tidy)

**What we defer to Phase 2:**
- Rename script (`script/rename_project.py`)
- CI/CD GitHub Actions workflow
- CPack workflow presets (packaging presets exist but workflow integration is
  minimal)
- Script consolidation/cleanup (moving from `utility/` to `script/`)

---

## 2. Proposed Changes

### 2.1 Directory Restructure (PRD §4, §9 Phase 1 Item 1)

**Current state:**
```
source/
└── myapp/
    ├── CMakeLists.txt
    └── main.cpp
```

**Target state:**
```
app/
└── sampleApp/
    ├── CMakeLists.txt
    └── src/
        └── main.cpp

lib/
└── logger/
    ├── CMakeLists.txt
    ├── include/
    │   └── logger/
    │       ├── ILogger.hpp
    │       └── LoggerFactory.hpp
    └── src/
        └── ConsoleLogger.cpp

test/
├── unit/
│   └── loggerUnitTest/
│       ├── CMakeLists.txt
│       └── src/
│           └── LoggerFactoryTest.cpp
└── int/
    └── appIntTest/
        ├── CMakeLists.txt
        └── src/
            └── AppIntTest.cpp
```

**Implementation steps:**

1. **Create new directory skeleton:**
   - Create `app/sampleApp/src/` directory
   - Create `lib/logger/{include/logger,src}` directories
   - Create `test/unit/loggerUnitTest/src/` directory
   - Create `test/int/appIntTest/src/` directory

2. **Migrate application:**
   - Move `source/myapp/main.cpp` → `app/sampleApp/src/main.cpp`
   - Create new `app/sampleApp/CMakeLists.txt` based on
     `source/myapp/CMakeLists.txt`
   - Update executable name from `myapp` to `sampleApp` (or `sample-app`)
   - Update main.cpp to use the new logger library (composition root)

3. **Create sample library (logger):**
   - Implement interface+factory pattern per PRD §6.3 example
   - Files:
     - `lib/logger/include/logger/ILogger.hpp` - abstract interface
     - `lib/logger/include/logger/LoggerFactory.hpp` - factory declaration
     - `lib/logger/src/ConsoleLogger.cpp` - concrete implementation +
       factory definition
   - Create `lib/logger/CMakeLists.txt` with:
     - `add_library(logger)` target
     - `FILE_SET HEADERS` for public headers
     - `target_include_directories()` with BUILD_INTERFACE and
       INSTALL_INTERFACE
     - `target_compile_features(... PRIVATE cxx_std_23)`
     - No fmt dependency (use iostream for simplicity)

4. **Remove old structure:**
   - Delete `source/` directory entirely after migration is verified

**Rationale:** The modular structure enables clear separation of concerns and
supports the PRD's encapsulation rules (public interfaces, private
implementations, composition root pattern).

---

### 2.2 Testing Infrastructure (PRD §5.4, §9 Phase 1 Item 5)

**Current state:** No testing infrastructure exists.

**Target state:** GoogleTest integrated via vcpkg with sample unit and
integration tests that demonstrate:
- Unit tests linking only the library-under-test
- Integration tests linking multiple internal libraries
- CTest integration with strict preset configuration

**Implementation steps:**

1. **Add GoogleTest dependency:**
   - Update `vcpkg.json`:
     ```json
     {
       "dependencies": [
         "fmt",
         "gtest"
       ]
     }
     ```

2. **Update root CMakeLists.txt:**
   - Add conditional testing setup:
     ```cmake
     if(BUILD_TESTING)
         enable_testing()
         add_subdirectory(test/unit/loggerUnitTest)
         add_subdirectory(test/int/appIntTest)
     endif()
     ```
   - Note: `BUILD_TESTING` is a standard CMake option set by `include(CTest)`
     (already present)

3. **Create unit test module (`test/unit/loggerUnitTest/`):**
   - `CMakeLists.txt`:
     ```cmake
     cmake_minimum_required(VERSION 3.28.3)

     set(TARGET_NAME loggerUnitTest)

     project(
         ${TARGET_NAME}
         VERSION 0.1.0
         DESCRIPTION "Unit tests for logger library."
         LANGUAGES CXX)

     # Add executable for unit tests
     add_executable(${TARGET_NAME})

     # Find GoogleTest
     find_package(GTest CONFIG REQUIRED)

     # Link to library-under-test and GoogleTest
     target_link_libraries(
         ${TARGET_NAME}
         PRIVATE logger
         PRIVATE GTest::gtest
         PRIVATE GTest::gtest_main)

     # Specify source files
     target_sources(
         ${TARGET_NAME}
         PRIVATE src/LoggerFactoryTest.cpp)

     # Set C++ standard
     target_compile_features(
         ${TARGET_NAME}
         PRIVATE cxx_std_23)

     # Compiler warnings
     target_compile_options(
         ${TARGET_NAME}
         PRIVATE
             $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra -Wpedantic -Werror>
             $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /WX>
     )

     # Set target properties
     set_target_properties(
         ${TARGET_NAME}
         PROPERTIES
         CXX_STANDARD 23
         CXX_STANDARD_REQUIRED TRUE
         CXX_EXTENSIONS FALSE)

     # Register with CTest
     include(GoogleTest)
     gtest_discover_tests(${TARGET_NAME})
     ```

   - `src/LoggerFactoryTest.cpp`:
     ```cpp
     #include <logger/ILogger.hpp>
     #include <logger/LoggerFactory.hpp>

     #include <gtest/gtest.h>

     #include <memory>
     #include <string_view>

     namespace sample::logger::test {

     namespace {

     /// Test suite for LoggerFactory.
     ///
     /// Validates factory functions that create logger instances.
     class LoggerFactoryTest : public ::testing::Test {
     protected:
       void SetUp() override {
         // No special setup needed for these tests
       }

       void TearDown() override {
         // No special teardown needed
       }
     };

     } // namespace

     // Test case: Factory creates a non-null logger instance
     TEST_F(LoggerFactoryTest, CreateDefaultLoggerReturnsNonNull)
     {
         auto logger = createDefaultLogger();
         ASSERT_NE(logger, nullptr) << "Factory should return a valid logger instance";
     }

     // Test case: Logger can log a simple message without throwing
     TEST_F(LoggerFactoryTest, CreatedLoggerCanLogMessage)
     {
         auto logger = createDefaultLogger();
         EXPECT_NO_THROW(logger->log("Test message"))
             << "Logger should handle basic message logging";
     }

     } // namespace sample::logger::test
     ```

4. **Create integration test module (`test/int/appIntTest/`):**
   - `CMakeLists.txt`: Similar structure to unit test, but links `logger`
     and tests end-to-end behaviour
     - For Phase 1, we create integration tests that verify logger + application
       interaction without linking the app executable directly (to avoid circular
       dependencies)
   - `src/AppIntTest.cpp`:
     ```cpp
     #include <logger/ILogger.hpp>
     #include <logger/LoggerFactory.hpp>

     #include <gtest/gtest.h>

     #include <memory>

     namespace sample::inttest {

     /// Test suite for application logging integration.
     ///
     /// These tests validate realistic logging scenarios that an application
     /// might encounter during normal operation.
     class AppLoggingIntSuite : public ::testing::Test {
     protected:
       std::unique_ptr<logger::ILogger> logger_;

       /// Set up a logger instance for integration testing.
       void SetUp() override { logger_ = logger::createDefaultLogger(); }

       /// Clean up logger instance after each test.
       void TearDown() override { logger_.reset(); }
     };

     // Test case: Scenario - Application logs startup sequence
     TEST_F(AppLoggingIntSuite, ApplicationStartupSequence)
     {
         // Scenario: Application starts and logs initialization steps
         ASSERT_NE(logger_, nullptr) << "Logger must be available for startup";

         EXPECT_NO_THROW({
             logger_->log("Application starting...");
             logger_->log("Loading configuration");
             logger_->log("Initializing subsystems");
             logger_->log("Application ready");
         });
     }

     } // namespace sample::inttest
     ```

5. **Update root CMakeLists.txt subdirectory declarations:**
   - Replace `add_subdirectory(source/myapp)` with:
     ```cmake
     add_subdirectory(lib/logger)
     add_subdirectory(app/sampleApp)
     ```

**Rationale:** Demonstrates the PRD's testing strategy (§5.4) with concrete
examples that template users can extend.

---

### 2.3 CMake Build System Updates (PRD §5.1, §9 Phase 1 Item 2)

**Current state:** Root CMakeLists.txt meets most PRD requirements but:
- Uses global `CMAKE_CXX_STANDARD` (PRD §5.1.6 requires per-target via
  `target_compile_features`)
- Does not include `test/` subdirectories
- References old `source/` directory

**Target state:** Fully PRD-compliant root build orchestrator.

**Implementation steps:**

1. **Remove global C++ standard setting:**
   - Delete these lines from root CMakeLists.txt:
     ```cmake
     # Set the C++ standard to C++23
     set(CMAKE_CXX_STANDARD 23)
     set(CMAKE_CXX_STANDARD_REQUIRED ON)
     set(CMAKE_CXX_EXTENSIONS OFF)
     ```
   - This ensures compliance with PRD §5.1.6: "Must express language standard
     requirements via target_compile_features(... PRIVATE cxx_std_23) not
     global CMAKE_CXX_STANDARD"
   - All targets already use `target_compile_features()`, so this change is
     safe

2. **Update subdirectory structure:**
   - Replace:
     ```cmake
     add_subdirectory(source/myapp)
     ```
   - With:
     ```cmake
     add_subdirectory(lib/logger)
     add_subdirectory(app/sampleApp)

     if(BUILD_TESTING)
         add_subdirectory(test/unit/loggerUnitTest)
         add_subdirectory(test/int/appIntTest)
     endif()
     ```

3. **Update CPack project name:**
   - Change `CPACK_PACKAGE_NAME` and root `project()` name from `cxxapp` to
     `cpp-app-template` (or similar) to match repository name
   - This ensures generated packages have meaningful names

**Rationale:** Ensures strict compliance with PRD's target-based architecture
requirements and correctly wires the new modular structure.

---

### 2.4 DevContainer Bootstrap Enhancement (PRD §5.3, §9 Phase 1 Item 6)

**Current state:** DevContainer exists but requires manual vcpkg bootstrap.

**Target state:** DevContainer automatically bootstraps vcpkg on creation and
documents the expected workflow.

**Implementation steps:**

1. **Update `devcontainer.json`:**
   - Add `postCreateCommand` that fails hard on bootstrap errors:
     ```jsonc
     {
       "name": "${localWorkspaceFolderBasename}-dev",
       "build": {
         "dockerfile": "Dockerfile",
         "context": "."
       },
       // ... existing configuration ...
       "postCreateCommand": "bash -c 'if [ ! -f ./vcpkg/vcpkg ]; then
         ./vcpkg/bootstrap-vcpkg.sh -disableMetrics; fi'",
       "containerEnv": {
         "VCPKG_ROOT":
           "${containerWorkspaceFolder}/vcpkg",
         "VCPKG_BINARY_SOURCES":
           "clear;files,${containerWorkspaceFolder}/vcpkg-cache,readwrite"
       },
       // ... existing customizations ...
     }
     ```

   **Note:** This command will fail container creation if vcpkg bootstrap
   fails, ensuring errors are visible immediately rather than being
   silently ignored.

2. **Document golden path in README:**
   - Add section describing the expected workflow:
     1. Open in DevContainer
     2. Run `cmake --preset debug`
     3. Run `cmake --build --preset debug-build`
     4. Run `ctest --preset debug-test --output-on-failure`

**Rationale:** Meets PRD requirement "Post-create bootstrap must ensure vcpkg
is available and bootstrapped at the pinned baseline" (§5.3.4) and improves
developer experience.

---

### 2.5 VS Code Configuration Updates (PRD §7.3)

**Current state:** `.vscode/tasks.json` and `.vscode/launch.json` exist and
use CMake Tools preset variables.

**Target state:** Configurations continue to work with the new directory
structure. Minor updates may be needed for launch target paths.

**Implementation steps:**

1. **Verify task configurations:**
   - Existing tasks use preset variables, so they should work unchanged
   - Test that "cmake: build all" and "cmake: test" tasks work after
     restructure

2. **Update launch configuration if needed:**
   - Current `launch.json` likely uses `${command:cmake.launchTargetPath}`
   - This should automatically resolve to the new `sampleApp` executable path
   - Test debugging to verify

3. **No changes expected** unless issues are discovered during testing

**Rationale:** Preset-driven configuration (as currently implemented) is
already compliant with PRD §7.3 requirements.

---

### 2.6 Documentation Updates (PRD Alignment)

**Current state:** README describes the old `source/myapp/` structure.

**Target state:** Documentation reflects the new modular layout and golden
path workflow.

**Implementation steps:**

1. **Update README.md:**
   - Replace directory structure diagram with new layout
   - Update "Getting Started" section with golden path commands
   - Document the module structure (app, lib, test/unit, test/int)
   - Add section on adding new libraries and tests

2. **Update PRD status:**
   - Mark Phase 1 deliverables as "Implemented" in PRD §9

3. **Create ARCHITECTURE.md:**
   - Document the interface+factory pattern
   - Explain composition root principle
   - Provide guidance on adding new modules

**Rationale:** Ensures template users understand the structure and can extend
it correctly.

---

### 2.7 Script Directory Migration (Partial, PRD §4)

**Current state:** Helper scripts live in `utility/`.

**Target state (PRD §4):** Scripts should live in `script/`.

**Implementation steps:**

1. **Create `script/` directory**
2. **Copy (don't move) existing scripts from `utility/` to `script/`:**
   - `clang-format-all.sh`
   - `clang-tidy-all.sh`
   - Keep `utility/` scripts as aliases or deprecation markers for Phase 2
3. **Update script paths in documentation**

**Rationale:** Partial migration preserves backward compatibility while moving
towards PRD compliance. Full migration (including vcpkg-setup.sh, etc.) is
deferred to Phase 2.

---

## 3. Dependencies

### 3.1 New Dependencies

| Dependency | Purpose | Integration Method |
|:-----------|:--------|:-------------------|
| GoogleTest (gtest) | Unit and integration testing framework | vcpkg manifest (`vcpkg.json`) + CMake `find_package(GTest CONFIG REQUIRED)` |

**Justification:** GoogleTest is the industry-standard C++ testing framework.
It integrates seamlessly with CMake via `gtest_discover_tests()` and is
available in vcpkg with pinned versioning.

### 3.2 Existing Dependencies (Retained)

| Dependency | Purpose | Notes |
|:-----------|:--------|:------|
| fmt | String formatting | Already in `vcpkg.json`, used by sampleApp |

---

## 4. Data Schema / API / Interface Contracts

### 4.1 Logger Interface Contract (lib/logger/include/logger/ILogger.hpp)

```cpp
namespace sample::logger {

/// @brief Abstract interface for logging messages.
///
/// Implementations must be thread-safe if used in multi-threaded contexts.
class ILogger {
public:
    virtual ~ILogger() = default;

    /// @brief Log a message.
    /// @param message The message to log. Must be valid UTF-8.
    virtual void log(std::string_view message) = 0;
};

} // namespace sample::logger
```

**Contract guarantees:**
- `log()` must not throw exceptions (implementations should catch and handle
  internally)
- `message` parameter is copied if needed; caller retains ownership
- Implementations must be safe to destroy after construction (no partial
  initialisation states)

### 4.2 Logger Factory Contract
(lib/logger/include/logger/LoggerFactory.hpp)

```cpp
namespace sample::logger {

/// @brief Create a default console logger.
///
/// @return A unique pointer to an ILogger implementation. Never returns
/// nullptr.
/// @throws std::runtime_error if logger creation fails (rare).
[[nodiscard]] std::unique_ptr<ILogger> createDefaultLogger();

} // namespace sample::logger
```

**Contract guarantees:**
- Returns a valid logger instance (never nullptr)
- Returned logger is heap-allocated and owned by caller
- Factory function is thread-safe (can be called from multiple threads)

### 4.3 CMake Target Interface Contract

**logger library target (PUBLIC interface):**
```cmake
add_library(logger)

# Public headers available to consumers
target_sources(logger
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/logger/ILogger.hpp
        include/logger/LoggerFactory.hpp
    PRIVATE
        src/ConsoleLogger.cpp)

# Public include paths
target_include_directories(logger
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>)
```

**Consumer expectations:**
- Can include `<logger/ILogger.hpp>` and `<logger/LoggerFactory.hpp>`
- Must link `PRIVATE logger` to access implementations
- No transitive dependencies (logger uses only standard library)

---

## 5. Observability & Testing

### 5.1 Testing Strategy

**Unit tests (`test/unit/loggerUnitTest/`):**
- **Purpose:** Verify logger library behaviour in isolation
- **Naming:** Executable target `loggerUnitTest`, test files `LoggerFactoryTest.cpp`, test suites `LoggerFactoryTest`
- **Linking:** Links only `logger` target and GoogleTest
- **Test cases (minimum):**
  1. `LoggerFactoryTest.CreateDefaultLoggerReturnsNonNull` - Factory returns non-null
  2. `LoggerFactoryTest.CreatedLoggerCanLogMessage` - Logger accepts messages without throwing
- **Advanced patterns:** Use GTest's value-parameterised tests (`TEST_P`, `INSTANTIATE_TEST_SUITE_P`) when testing a method with multiple input scenarios

**Integration tests (`test/int/appIntTest/`):**
- **Purpose:** Verify end-to-end behaviour (app + library interaction)
- **Naming:** Executable target `appIntTest`, test files `AppIntTest.cpp`, test suites `AppLoggingIntSuite`
- **Linking:** Links `logger` target and GoogleTest (simulates app usage)
- **Test cases (minimum):**
  1. `AppLoggingIntSuite.ApplicationStartupSequence` - Realistic usage scenario

### 5.2 CTest Configuration

**Test presets (`CMakePresets.json`):**
```json
"testPresets": [
    {
        "name": "base-test",
        "hidden": true,
        "output": {
            "outputOnFailure": true,
            "verbosity": "verbose"
        },
        "execution": {
            "noTestsAction": "error",
            "stopOnFailure": true
        }
    }
]
```

**Compliance with PRD §5.4.4:**
- ✅ `outputOnFailure: true` - Shows test output immediately on failure
- ✅ `stopOnFailure: true` - Fails fast, no wasted CI time
- ✅ `noTestsAction: error` - Catches missing test registration bugs

### 5.3 Manual Testing Checklist

After implementation, verify:
1. ✅ Clean configure: `cmake --preset debug` succeeds
2. ✅ Build: `cmake --build --preset debug-build` succeeds
3. ✅ Test: `ctest --preset debug-test --output-on-failure` succeeds (all
   tests pass)
4. ✅ Run app: `./build/debug/app/sampleApp/sampleApp` runs and logs messages
5. ✅ Lint: `cmake --preset debug-clang-tidy && cmake --build --preset
   debug-clang-tidy-build` succeeds (no warnings)
6. ✅ DevContainer: Open in DevContainer, vcpkg bootstraps automatically

---

## 6. Key Decisions and Assumptions

### 6.1 Design Decisions

| Decision | Rationale | Alternatives Considered |
|:---------|:----------|:------------------------|
| Use interface+factory pattern for logger | Demonstrates PRD §6.3 encapsulation pattern; provides concrete example for template users | Simple header-only logger (rejected: doesn't demonstrate library structure) |
| Keep sample library minimal (no dependencies) | Reduces vcpkg download time; focuses on structure over functionality | Use spdlog or similar (rejected: adds complexity, increases barrier to understanding) |
| Create both unit and integration tests | Demonstrates PRD §5.4 distinction between test types | Only unit tests (rejected: PRD requires both) |
| Copy scripts to `script/` rather than moving | Preserves backward compatibility for Phase 1 | Move immediately (rejected: may break user workflows) |
| Name executable `sampleApp` (PascalCase) | Follows PRD §6.1 naming conventions for type-like entities | `sample-app` (kebab-case, rejected: inconsistent with codebase style) |
| Use `sample::logger` namespace | Shorter, clearly example code; easy to search/replace | `myapp::logger` (rejected: less clear as placeholder), `cppAppTemplate::logger` (rejected: too verbose) |
| Name test executables with PascalCase + standardized suffixes | Clear, consistent pattern: `<lib-name>UnitTest` for unit tests, `<component-name>IntTest` for integration tests | camelCase (rejected: less readable for multi-word names), snake_case (rejected: inconsistent with codebase) |
| Name test files with PascalCase matching class under test | Clear mapping between test file and tested component (e.g., `LoggerFactoryTest.cpp` tests `LoggerFactory`) | camelCase or snake_case (rejected: inconsistent with codebase file naming) |
| Name integration test suites with descriptive names + `IntSuite` suffix | Clearly distinguishes integration test suites from unit test suites | Using `Test` suffix for integration suites (rejected: ambiguous with unit tests) |
| Fail hard on DevContainer vcpkg bootstrap errors | Ensures problems are caught immediately; prevents silent failures | Fail silently (rejected: may hide critical issues) |
| Add ci-linux preset in Phase 1 | Provides complete preset configuration ready for Phase 2 CI work | Defer to Phase 2 (rejected: requires preset changes later) |
| Broader ARCHITECTURE.md scope | Educational value for template users; documents SOLID principles and DI patterns | Minimal scope (rejected: misses opportunity to educate) |
| No migration support for existing users | Template repositories are starting points, not upgraded codebases | Provide migration script (rejected: significant additional work for limited value) |

### 6.2 Assumptions

1. **vcpkg baseline is stable:** The pinned baseline
   (`2e6fcc44573d091af0321f99c89b212997a76f1f`) provides a working GoogleTest
   version. If not, the baseline will need to be updated.

2. **BUILD_TESTING default is ON:** Standard CMake behaviour from
   `include(CTest)` sets `BUILD_TESTING=ON` by default. Users can disable via
   `-DBUILD_TESTING=OFF`.

3. **Linux-only assumption holds:** No platform-specific CMake or code changes
   are needed. All testing occurs on Linux (Ubuntu 24.04 LTS in DevContainer).

4. **Namespace consolidation:** New code uses `sample::logger` namespace
   (short and clearly example code). PRD allows this to be renamed via future
   rename script, so the specific name is not critical.

5. **No interface changes to existing presets:** Current presets (debug,
   debug-clang-tidy, release) remain unchanged. Users with local overrides in
   `CMakeUserPresets.json` will not be disrupted.

### 6.3 Trade-offs

| Trade-off | Choice | Impact |
|:----------|:-------|:-------|
| Sample library complexity | Minimal (console logger only) | Faster to implement, easier to understand; users must add domain logic themselves |
| Test coverage | Minimal (2 unit tests, 1 integration test) | Demonstrates patterns without overwhelming template users with extensive test code |
| Migration approach | Big bang (restructure all at once) | Single PR, easier to review atomically; more disruptive than incremental migration |
| Script directory migration | Partial (copy, not move) | Backward compatible but creates temporary duplication |

---

## 7. Tasks

All tasks are estimated in hours for a single experienced developer.

### 7.1 Phase 1.1: Directory Structure & Core Library (16 hours)

| # | Task | Description | Effort (h) | Dependencies |
|:--|:-----|:------------|:-----------|:-------------|
| 1.1.1 | Create directory skeleton | Create `app/`, `lib/`, `test/unit/`, `test/int/` with subdirectories | 0.5 | None |
| 1.1.2 | Implement ILogger interface | Create `lib/logger/include/logger/ILogger.hpp` with abstract interface | 1 | 1.1.1 |
| 1.1.3 | Implement LoggerFactory | Create `lib/logger/include/logger/LoggerFactory.hpp` (header) | 0.5 | 1.1.2 |
| 1.1.4 | Implement ConsoleLogger | Create `lib/logger/src/ConsoleLogger.cpp` with concrete implementation and factory definition | 2 | 1.1.3 |
| 1.1.5 | Create logger CMakeLists.txt | Create `lib/logger/CMakeLists.txt` with target-based config, FILE_SET HEADERS | 2 | 1.1.4 |
| 1.1.6 | Migrate app to app/sampleApp | Copy `source/myapp/main.cpp` to `app/sampleApp/src/main.cpp`, update to use logger | 1.5 | 1.1.5 |
| 1.1.7 | Create app CMakeLists.txt | Create `app/sampleApp/CMakeLists.txt`, link to logger library | 1.5 | 1.1.6 |
| 1.1.8 | Update root CMakeLists.txt | Remove global CXX_STANDARD, update subdirectories, conditional testing block | 2 | 1.1.7 |
| 1.1.9 | Manual smoke test | Configure, build, run sampleApp to verify basic functionality | 1 | 1.1.8 |
| 1.1.10 | Delete source/ directory | Remove old structure after verification | 0.5 | 1.1.9 |
| **Subtotal** | | | **12.5** | |

### 7.2 Phase 1.2: Testing Infrastructure (12 hours)

| # | Task | Description | Effort (h) | Dependencies |
|:--|:-----|:------------|:-----------|:-------------|
| 1.2.1 | Add GoogleTest to vcpkg.json | Update dependency manifest | 0.5 | None |
| 1.2.2 | Create unit test structure | Create `test/unit/loggerUnitTest/src/` directory | 0.25 | 1.1.1 |
| 1.2.3 | Implement unit test cases | Create `LoggerFactoryTest.cpp` with Factory tests | 2 | 1.1.5, 1.2.2 |
| 1.2.4 | Create unit test CMakeLists.txt | Create `test/unit/loggerUnitTest/CMakeLists.txt` with gtest_discover_tests | 2 | 1.2.3 |
| 1.2.5 | Create integration test structure | Create `test/int/appIntTest/src/` directory | 0.25 | 1.1.1 |
| 1.2.6 | Implement integration test | Create `AppIntTest.cpp` with end-to-end scenario | 1.5 | 1.1.7, 1.2.5 |
| 1.2.7 | Create integration test CMakeLists.txt | Create `test/int/appIntTest/CMakeLists.txt` | 1.5 | 1.2.6 |
| 1.2.8 | Wire tests in root CMakeLists | Add `add_subdirectory()` calls for test modules | 0.5 | 1.2.4, 1.2.7 |
| 1.2.9 | Configure and build tests | Run `cmake --preset debug`, `cmake --build --preset debug-build` | 0.5 | 1.2.8 |
| 1.2.10 | Run tests and verify | Run `ctest --preset debug-test`, ensure all pass | 1 | 1.2.9 |
| 1.2.11 | Test debug and release presets | Verify tests pass in both configurations | 1 | 1.2.10 |
| **Subtotal** | | | **11** | |

### 7.3 Phase 1.3: DevContainer & Developer Experience (6 hours)

| # | Task | Description | Effort (h) | Dependencies |
|:--|:-----|:------------|:-----------|:-------------|
| 1.3.1 | Update devcontainer.json | Add postCreateCommand for vcpkg bootstrap, add containerEnv | 1 | None |
| 1.3.2 | Test DevContainer bootstrap | Rebuild container, verify vcpkg bootstraps automatically | 0.5 | 1.3.1 |
| 1.3.3 | Test golden path in DevContainer | Run full configure/build/test cycle inside container | 1 | 1.3.2 |
| 1.3.4 | Verify VS Code tasks | Test "cmake: build all", "cmake: test" tasks work with new structure | 0.5 | 1.1.8 |
| 1.3.5 | Test debugging in DevContainer | Launch sampleApp via F5, verify breakpoints work | 1 | 1.1.9 |
| 1.3.6 | Document troubleshooting | Add DevContainer troubleshooting section to README | 1 | 1.3.3 |
| **Subtotal** | | | **5** | |

### 7.4 Phase 1.4: Documentation & Cleanup (8 hours)

| # | Task | Description | Effort (h) | Dependencies |
|:--|:-----|:------------|:-----------|:-------------|
| 1.4.1 | Update README.md structure | Replace directory diagram, update Getting Started | 2 | 1.1.10 |
| 1.4.2 | Document adding new libraries | Add section on creating lib/<name> modules | 1.5 | 1.4.1 |
| 1.4.3 | Document adding new tests | Add section on creating unit and integration tests | 1.5 | 1.4.1 |
| 1.4.4 | Create script/ directory | Create `script/` and copy clang-format-all.sh, clang-tidy-all.sh | 0.5 | None |
| 1.4.5 | Update script paths in README | Update documentation to reference `script/` instead of `utility/` | 0.5 | 1.4.4 |
| 1.4.6 | Create ARCHITECTURE.md | Document interface+factory pattern, composition root, dependency injection principles, and SOLID principles | 3 | 1.1.10 |
| 1.4.7 | Update PRD status | Mark Phase 1 as "Implemented" in doc/PRD.md | 0.5 | All tasks |
| 1.4.8 | Add ci-linux preset | Add ci-linux configure preset to CMakePresets.json with CI-optimized settings | 0.5 | None |
| **Subtotal** | | | **10** | |

### 7.5 Phase 1.5: Validation & Polish (6 hours)

| # | Task | Description | Effort (h) | Dependencies |
|:--|:-----|:------------|:-----------|:-------------|
| 1.5.1 | Clean room test | Clone fresh, open in DevContainer, run golden path | 1 | All tasks |
| 1.5.2 | Run clang-tidy preset | `cmake --preset debug-clang-tidy`, build, verify no warnings | 1 | 1.1.10 |
| 1.5.3 | Run clang-format | Verify all files are formatted per .clang-format | 0.5 | 1.1.10 |
| 1.5.4 | Test package generation | Run `cpack --preset release-package`, verify TGZ created | 0.5 | 1.1.8 |
| 1.5.5 | Test workflow presets | Run `cmake --workflow --preset debug-workflow`, verify end-to-end | 1 | All tasks |
| 1.5.6 | Review and fix issues | Address any issues found during validation | 2 | 1.5.1-1.5.5 |
| **Subtotal** | | | **6** | |

### 7.6 Summary

| Phase | Tasks | Total Effort (hours) |
|:------|:------|:---------------------|
| 1.1 Directory Structure & Core Library | 10 | 12.5 |
| 1.2 Testing Infrastructure | 11 | 11 |
| 1.3 DevContainer & Developer Experience | 6 | 5 |
| 1.4 Documentation & Cleanup | 8 | 10 |
| 1.5 Validation & Polish | 6 | 6 |
| **Total** | **41** | **44.5** |

**Estimated delivery time:** 1 week (assuming 1 developer working 6-8
hours/day, approximately 44.5 hours total)

---

## 8. Implementation Order

**Recommended sequence:**
1. **Phase 1.1** (Directory Structure & Core Library) - Establishes foundation
2. **Phase 1.2** (Testing Infrastructure) - Adds tests before extensive
   validation
3. **Phase 1.3** (DevContainer & Developer Experience) - Verifies golden path
4. **Phase 1.4** (Documentation & Cleanup) - Makes template usable, adds
   ci-linux preset
5. **Phase 1.5** (Validation & Polish) - Final verification

**Rationale:** Bottom-up approach ensures each layer is stable before building
on it. Documentation comes after implementation to accurately reflect the final
state. The ci-linux preset is added in Phase 1.4 to provide a complete preset
configuration ready for Phase 2 CI work.

---

## 9. Risks & Mitigations

| Risk | Likelihood | Impact | Mitigation |
|:-----|:-----------|:-------|:-----------|
| GoogleTest vcpkg version incompatibility | Low | Medium | Test GoogleTest installation early (Task 1.2.1); update vcpkg baseline if needed |
| CMake target dependency ordering issues | Medium | High | Build incrementally (complete Phase 1.1 before 1.2); use `cmake --graphviz` to visualise dependencies |
| DevContainer vcpkg bootstrap failure | Low | Medium | Test bootstrap explicitly (Task 1.3.2); add error handling to postCreateCommand |
| Breaking changes to existing user workflows | Medium | High | Preserve presets, provide migration guide; defer rename script to Phase 2 |
| Test discovery failure (gtest_discover_tests) | Low | Medium | Use `--rerun-failed --output-on-failure` during testing; check CTest verbose output |

---

## 10. Success Criteria

Phase 1 is considered complete when:

1. ✅ **Structural requirements (PRD §4):**
   - `app/`, `lib/`, `test/unit/`, `test/int/` directories exist with sample
     modules
   - `source/` directory is deleted

2. ✅ **Build requirements (PRD §5.1, §9 Phase 1 Item 2):**
   - Root CMakeLists.txt uses target-based architecture only (no global
     mutations)
   - All targets use `target_compile_features(... PRIVATE cxx_std_23)`
   - `cmake --preset debug` succeeds
   - `cmake --build --preset debug-build` succeeds with zero warnings

3. ✅ **Testing requirements (PRD §5.4, §9 Phase 1 Item 5):**
   - GoogleTest is integrated via vcpkg
   - At least one unit test exists and passes
   - At least one integration test exists and passes
   - `ctest --preset debug-test --output-on-failure` succeeds
   - Test presets enforce `noTestsAction: error`, `stopOnFailure: true`,
     `outputOnFailure: true`

4. ✅ **DevContainer requirements (PRD §5.3, §9 Phase 1 Item 6):**
   - Opening repo in DevContainer bootstraps vcpkg automatically
   - Full configure/build/test cycle works without manual intervention

5. ✅ **Documentation requirements:**
   - README reflects new directory structure
   - Golden path workflow is documented
   - Example of adding new libraries and tests is provided

6. ✅ **Quality requirements:**
   - `cmake --preset debug-clang-tidy && cmake --build --preset
     debug-clang-tidy-build` succeeds (no clang-tidy warnings)
   - All code is formatted per `.clang-format`

7. ✅ **Acceptance criteria (PRD §9 Phase 1):**
   - Clean clone → configure → build → test succeeds on Linux
   - Sample app compiles and runs
   - No in-source build possible (error message shown)
   - Dependency resolution via vcpkg manifest mode (no ambient packages
     required)

---

## Appendix A: File Inventory

### A.1 Files to Create

| Path | Type | Description |
|:-----|:-----|:------------|
| `app/sampleApp/CMakeLists.txt` | CMake | Application build configuration |
| `app/sampleApp/src/main.cpp` | C++ | Application entry point (composition root) |
| `lib/logger/CMakeLists.txt` | CMake | Library build configuration |
| `lib/logger/include/logger/ILogger.hpp` | C++ | Logger interface |
| `lib/logger/include/logger/LoggerFactory.hpp` | C++ | Factory declaration |
| `lib/logger/src/ConsoleLogger.cpp` | C++ | Console logger implementation + factory |
| `test/unit/loggerUnitTest/CMakeLists.txt` | CMake | Unit test build configuration |
| `test/unit/loggerUnitTest/src/LoggerFactoryTest.cpp` | C++ | Unit tests for logger factory |
| `test/int/appIntTest/CMakeLists.txt` | CMake | Integration test build configuration |
| `test/int/appIntTest/src/AppIntTest.cpp` | C++ | Integration tests for application |
| `script/clang-format-all.sh` | Bash | Format all C++ files (copy from utility/) |
| `script/clang-tidy-all.sh` | Bash | Lint all C++ files (copy from utility/) |
| `doc/ARCHITECTURE.md` | Markdown | Architecture documentation (interface+factory pattern, DI, SOLID principles) |

### A.2 Files to Modify

| Path | Changes |
|:-----|:--------|
| `CMakeLists.txt` | Remove global CXX_STANDARD; update subdirectories; add test conditional |
| `vcpkg.json` | Add gtest dependency |
| `.devcontainer/devcontainer.json` | Add postCreateCommand, containerEnv |
| `README.md` | Update structure diagram, Getting Started, add module guidance |
| `doc/PRD.md` | Mark Phase 1 as "Implemented" |

### A.3 Files to Delete

| Path | Reason |
|:-----|:-------|
| `source/` (entire directory) | Replaced by modular `app/` and `lib/` structure |

---

## Appendix B: Example Code Snippets

### B.1 Sample app/sampleApp/src/main.cpp (Composition Root)

```cpp
#include <logger/LoggerFactory.hpp>

#include <cstdlib>

/**
 * @brief Application entry point (composition root).
 *
 * Wires dependencies and delegates to application logic.
 *
 * @return EXIT_SUCCESS on success.
 */
auto
main() -> int
{
    // Composition root: Create dependencies
    auto logger = sample::logger::createDefaultLogger();

    // Application logic
    logger->log("Application started");
    logger->log("Hello from cpp-app-template!");
    logger->log("Application finished");

    return EXIT_SUCCESS;
}
```

### B.2 Sample lib/logger/include/logger/ILogger.hpp

```cpp
#pragma once

#include <string_view>

namespace sample::logger {

/// @brief Abstract interface for logging messages.
///
/// Implementations must be thread-safe if used in multi-threaded
/// contexts.
class ILogger {
public:
    virtual ~ILogger() = default;

    /// @brief Log a message.
    ///
    /// @param message The message to log. Must be valid UTF-8.
    virtual void log(std::string_view message) = 0;
};

} // namespace sample::logger
```

### B.3 Sample lib/logger/src/ConsoleLogger.cpp

```cpp
#include <logger/ILogger.hpp>
#include <logger/LoggerFactory.hpp>

#include <iostream>
#include <memory>

namespace sample::logger {

namespace {

/// @brief Console logger implementation (private).
///
/// Writes log messages to stdout.
class ConsoleLogger final : public ILogger {
public:
    void log(std::string_view message) override
    {
        std::cout << message << '\n';
    }
};

} // anonymous namespace

std::unique_ptr<ILogger>
createDefaultLogger()
{
    return std::make_unique<ConsoleLogger>();
}

} // namespace sample::logger
```

---

## Appendix C: CMake Target Dependency Graph

```
sampleApp (executable)
  └── logger (library)

loggerUnitTest (executable)
  └── logger (library)
  └── GTest::gtest
  └── GTest::gtest_main

appIntTest (executable)
  └── logger (library)
  └── GTest::gtest
  └── GTest::gtest_main
```

**Notes:**
- Arrows indicate link dependencies
- All targets implicitly depend on vcpkg-provided dependencies (fmt for
  sampleApp, gtest for tests)
- No circular dependencies exist

---

## Appendix D: vcpkg Dependency Resolution

**Manifest file (`vcpkg.json`):**
```json
{
  "dependencies": [
    "fmt",
    "gtest"
  ]
}
```

**Baseline pinning (`vcpkg-configuration.json`):**
```json
{
  "default-registry": {
    "kind": "git",
    "baseline": "2e6fcc44573d091af0321f99c89b212997a76f1f",
    "repository": "https://github.com/microsoft/vcpkg"
  }
}
```

**Resolution order:**
1. vcpkg reads `vcpkg.json` for dependency list
2. vcpkg uses baseline from `vcpkg-configuration.json` to resolve versions
3. vcpkg checks `vcpkg-cache/` for pre-built binaries
4. If not cached, vcpkg builds from source and caches result
5. CMake `find_package()` locates vcpkg-installed packages

**Expected install location:**
- Debug: `build/debug/vcpkg_installed/arm64-linux-gnu/`
- Release: `build/release/vcpkg_installed/arm64-linux-gnu/`

---

**End of Implementation Plan**
