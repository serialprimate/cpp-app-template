# Instructions for GitHub Copilot

## Project Overview

**cpp-app-template** is a modern C++23 application template built with CMake and vcpkg. This template enforces production-ready practices, reproducible builds, and modular architecture.

**Platform Support**: This template supports **Linux only**. Support for other operating systems is not provided.

**Current State**: The project implements a minimal working template with:
- Single application structure (`app/sampleApp/` with `main.cpp`)
- vcpkg manifest mode with a pinned `builtin-baseline` (in `vcpkg.json`)
- CMake presets for lint, debug, release, and CI builds
- Workflow presets chaining configure → build → test → package
- Build hygiene (out-of-source enforcement, sanitiser guards for release builds)
- DevContainer support

### Architecture & Structure

**Current Layout:**
- **Root CMakeLists.txt**: Orchestrates the build, enforces out-of-source builds, configures sanitisers (non-release build types only), and sets up CPack packaging
- **app/**: Contains application modules (currently `sampleApp/` with `main.cpp`)
- **external/vcpkg/**: vcpkg submodule for dependency management (manifest mode)
- **vcpkg/**: Project-owned vcpkg configuration (cache + overlay triplets)
- **script/**: Helper scripts for formatting, linting, and vcpkg lifecycle
- **doc/**: Project documentation including ARCHITECTURE.md
- **build/**: Generated build artifacts (never committed)
- **vcpkg/cache/**: Local binary cache for vcpkg artifacts (not committed)
- **vcpkg/triplets/**: Overlay triplets for custom toolchain configuration
- **Custom triplets**: Uses `vcpkg/triplets/arm64-linux-gnu.cmake` with GNU toolchain overlay

## Critical Workflows

### Building & Testing

```bash
# Configure (select preset: lint, debug, release, ci)
cmake --preset debug

# Build
cmake --build --preset debug-build

# Test (when tests exist)
ctest --preset debug-test --output-on-failure

# Clean rebuild
cmake --build --preset debug-rebuild

# Package
cpack --preset debug-package

# Full workflow (configure + build + test + package)
cmake --workflow --preset debug-workflow
```

Use VS Code tasks for convenience: "cmake: build all", "cmake: test", "cmake: configure"

**Available CMake Presets:**
- **Configure**: `lint`, `debug`, `release`, `ci` (plus `*-arm64` and `*-x64` variants)
- **Build**: `lint-build`, `lint-rebuild`, `debug-build`, `debug-rebuild`, `release-build`, `release-rebuild`, `ci-build`
- **Test**: `debug-test`, `release-test`, `ci-test`
- **Package**: `debug-package`, `release-package`, `ci-package` (TGZ)
- **Workflow**: `debug-workflow`, `release-workflow`, `ci-workflow` (configure → build → test → package)

### VS Code (Current)

- Tasks live in `.vscode/tasks.json` and are driven by the active CMake preset (CMake Tools).
- Debugging lives in `.vscode/launch.json` and resolves the executable via CMake Tools (launch target path).

### Dependency Management

- vcpkg is tracked as a git submodule at `external/vcpkg/`
- Add dependencies to `vcpkg.json` (manifest file)
- Use `find_package(<Pkg> CONFIG REQUIRED)` in CMakeLists.txt
- Link via the package's exported CMake targets (examples: `fmt::fmt`, `GTest::gtest`, `GTest::gtest_main`). Target names vary by package.
- Submodule initialisation: `git submodule update --init --recursive`
- vcpkg bootstrap: `./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics`
- Or use convenience script: `./script/vcpkg-bootstrap.sh`

### Architecture Selection

This template is configured around vcpkg triplets and matching CMake presets:
- vcpkg triplets: `arm64-linux-gnu` (overlay triplet in `vcpkg/triplets/`) and `x64-linux` (built-in vcpkg triplet)
- CMake configure presets: `*-arm64` and `*-x64` variants (the `*-x64` variants are labelled **UNTESTED** in the preset metadata)
- Default configure presets (`lint`, `debug`, `release`, `ci`) inherit from the ARM64 variants
- Select an architecture explicitly when needed: `cmake --preset debug-arm64` or `cmake --preset debug-x64`

### Code Quality Tools

```bash
# Format all C++ files (uses the repo's .clang-format)
./script/clang-format-all.sh

# Lint all C++ files (clang-tidy reads the repo's .clang-tidy)
./script/clang-tidy-all.sh
```

Use `lint` preset to enable clang-tidy during build.

Note: `./script/clang-tidy-all.sh` runs clang-tidy with `-p build/debug`. Ensure you've configured `debug` at least once (e.g. `cmake --preset debug`) before running it, or adjust the script/build directory.

## Project-Specific Conventions

### Naming (LLVM/Clang-aligned with PascalCase filenames)

- **Files defining types**: PascalCase (e.g., `MappedFile.hpp`, `MappedFile.cpp`)
- **Utility/test files**: PascalCase with descriptive suffix (e.g., `LoggerFactoryTest.cpp`, `AppIntegrationTest.cpp`)
- **Types**: PascalCase (e.g., `MappedFile`, `ExitCode`)
- **Functions**: camelCase (e.g., `openReadonly()`, `printUsage()`)
- **Variables**: camelCase (e.g., `filePath`, `bufferSize`)
- **Constants**: PascalCase or ALL_CAPS
- **Namespaces**: camelCase (e.g., `myProj`, `testHelpers`)
- **CMake targets/executables**: lowerCamelCase with standardised suffixes:
  - Unit tests: `<lib-name>UnitTest` (e.g., `loggerUnitTest`)
  - Integration tests: `<component-name>IntTest` (e.g., `appIntTest`)
- **Test suites (GTest fixtures)**: PascalCase with descriptive suffix:
  - Unit tests: `<class-name>Test` (e.g., `LoggerFactoryTest`, `ConsoleLoggerTest`)
  - Integration tests: Descriptive name ending in `Suite` using `Int` abbreviation (e.g., `AppLoggingIntSuite`, `AppLifecycleSuite`)

### C++ Standards

- **C++23** is the default language standard
- Use modern idioms: RAII, smart pointers, `std::format`, concepts, ranges
- Compiler warnings are treated as errors by default on targets in this repo (e.g. `-Wall -Wextra -Wpedantic -Werror` for GNU/Clang)

### CMake Patterns

- **Minimum version**: CMake 3.28.3 (enforced at root)
- **Target-based architecture**: Model all build artefacts as targets, express all requirements via target commands
- **Scope discipline**: Use correct CMake target scopes:
  - `PRIVATE`: Implementation-only (compile options, private deps, internal includes)
  - `PUBLIC`: Required by target and consumers (transitive deps, public API)
  - `INTERFACE`: Required only by consumers (header-only libs, usage requirements)
- Prefer target-based configuration. Avoid global state (`include_directories()`, `link_libraries()`, `add_definitions()`, global `CMAKE_CXX_FLAGS`).
- Express language standard via `target_compile_features(... PRIVATE cxx_std_23)`, not global `CMAKE_CXX_STANDARD`
- Keep CMakeLists.txt structure uniform across all modules
- Prefer CMake Presets over cache variables for developer options

### Testing Conventions

#### Unit Tests
- **Executable naming**: `<lib-name>UnitTest` (e.g., `loggerUnitTest`, `dataProcessorUnitTest`)
- **Location**: `test/unit/<target-name>/` (e.g., `test/unit/loggerUnitTest/`)
- **File naming**: PascalCase matching class under test (e.g., `LoggerFactoryTest.cpp`)
- **Test suite naming**: `<class-name>Test` (e.g., `class LoggerFactoryTest : public ::testing::Test`)
- **Test case focus**: Test individual methods or behaviours of a class
- **Linking**: Link only the library-under-test and its declared public dependencies
- **Parameterised tests**: Use GTest's value-parameterised tests (`TEST_P`, `INSTANTIATE_TEST_SUITE_P`) when testing a method with multiple input scenarios

#### Integration Tests
- **Executable naming**: `<component-name>IntTest` (e.g., `appIntTest`, `databaseIntTest`)
- **Location**: `test/int/<target-name>/` (e.g., `test/int/appIntTest/`)
- **File naming**: PascalCase matching target name (e.g., `AppIntTest.cpp`)
- **Test suite naming**: Descriptive name ending in `Suite` using `Int` abbreviation (e.g., `class AppLoggingIntSuite : public ::testing::Test`)
- **Test case focus**: Represent realistic end-to-end scenarios (e.g., "ApplicationStartupSequence", "HighVolumeLoggingScenario")
- **Linking**: May link multiple internal libraries
- **Structure**: Each test suite is a collection of related use-cases; each test case is an individual scenario

**See ARCHITECTURE.md "Adding Tests" for complete testing requirements and examples.**

### Build Hygiene

- Out-of-source builds enforced (root CMakeLists.txt checks and errors)
- Sanitisers (`ENABLE_ASAN`, `ENABLE_UBSAN`, `ENABLE_TSAN`) are guarded so they cannot be enabled for `Release`/`MinSizeRel` build types
- `CMAKE_EXPORT_COMPILE_COMMANDS=ON` for IDE/tool integration (compile_commands.json)
- vcpkg binary caching via `VCPKG_BINARY_SOURCES="clear;files,${sourceDir}/vcpkg/cache,readwrite"`
- vcpkg metrics disabled: `VCPKG_BOOTSTRAP_OPTIONS="-disableMetrics"`
- vcpkg install options: `VCPKG_INSTALL_OPTIONS="--clean-after-build"` (keeps builds lean)

## Language-Specific Instructions

This project uses language-specific instruction files that AI agents **must consult** before editing code:

- **C/C++**: `.github/instructions/cpp.instructions.md` - C++23 patterns, memory management, type safety
- **CMake**: `.github/instructions/cmake.instructions.md` - Modern CMake practices, target-based builds
- **Bash**: `.github/instructions/bash.instructions.md` - Strict mode, quoting, modern idioms
- **Python**: `.github/instructions/python.instructions.md` - Type hints, Python 3.12+ features

Always read the applicable instruction file before creating or modifying files of that type.

## Common Pitfalls to Avoid

- ❌ Don't hardcode paths or use `${CMAKE_SOURCE_DIR}` indiscriminately (use `${CMAKE_CURRENT_SOURCE_DIR}`)
- ❌ Don't enable sanitizers in release builds (CMakeLists.txt guards this)
- ❌ Don't bypass vcpkg (no manual downloads or vendored dependencies)
- ❌ Don't use raw pointers for ownership (use `std::unique_ptr` or `std::shared_ptr`)
- ❌ Don't sprinkle `#include` without forward declarations
- ❌ Don't ignore preset naming (base presets are `hidden: true`, only concrete presets are selectable)
- ❌ Don't use global CMake mutations - use modern CMake target-based configurations
- ❌ Don't set `CMAKE_CXX_STANDARD` globally - use `target_compile_features(... PRIVATE cxx_std_23)`
- ❌ Don't commit `build/`, `vcpkg/cache/`, or `CMakeUserPresets.json`
- ❌ Don't forget to initialise submodules after cloning (`git submodule update --init --recursive`)
- ❌ Don't use untested x64 presets without verifying on x64 hardware
- ❌ Don't move vcpkg overlay triplets out of `vcpkg/triplets/` (breaks preset paths)

## Quick Reference

| Task | Command |
|------|---------|
| Bootstrap vcpkg | `./script/vcpkg-bootstrap.sh` |
| Configure debug | `cmake --preset debug` |
| Configure CI | `cmake --preset ci` |
| Build | `cmake --build --preset debug-build` |
| Test | `ctest --preset debug-test --output-on-failure` |
| Package | `cpack --preset release-package` |
| Full workflow | `cmake --workflow --preset debug-workflow` |
| Format code | `./script/clang-format-all.sh` |
| Lint code | `./script/clang-tidy-all.sh` |
| Add dependency | Edit `vcpkg.json`, then reconfigure |

## General Instructions

- Always check syntax, style, completeness, consistency, and logic errors
- Follow all language-specific instructions from `.github/instructions/*.md`
- Use Australian English spelling and grammar
- Comment code thoroughly with Doxygen-compatible documentation
- You are working in a DevContainer, ensure that the configuration is maintained
- Prefer explicitness and clarity over brevity in code, configurations, and documentation
