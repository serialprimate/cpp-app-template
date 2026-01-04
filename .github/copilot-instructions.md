# Instructions for GitHub Copilot

## Project Overview

**cpp-app-template** is a modern C++23 application template built with CMake and vcpkg. This template enforces production-ready practices, reproducible builds, and modular architecture.

**Platform Support**: This template supports **Linux only**. Support for other operating systems is not provided or planned.

**Current State**: The project implements a minimal working template with:
- Single application structure (`app/sampleApp/` with `main.cpp`)
- vcpkg manifest mode with pinned baseline (`vcpkg.json`, `vcpkg-configuration.json`)
- CMake presets for lint, debug, and release builds
- Workflow presets chaining configure → build → test → package
- Build hygiene (out-of-source enforcement, sanitiser guards)
- Basic DevContainer support

**Future Architecture**: See `doc/PRD.md` (v2.0.0 Draft) for the complete target specification including:
- Modular layout (`app/`, `lib/`, `test/unit/`, `test/int/`)
- Interface+factory pattern for encapsulation (PRD section 6)
- GoogleTest integration and testing strategy (PRD section 5.4)
- DevContainer and CI/CD setup (PRD sections 5.3, 8)
- Rename script and project initialization workflow (PRD section 7)

### Architecture & Structure

**Current Layout:**
- **Root CMakeLists.txt**: Orchestrates the build, enforces out-of-source builds, configures sanitizers (debug only), and sets up CPack packaging
- **app/**: Contains application modules (currently `sampleApp/` with `main.cpp`)
- **external/vcpkg/**: vcpkg submodule for dependency management (manifest mode)
- **vcpkg/**: Project-owned vcpkg configuration (cache + overlay triplets)
- **script/**: Helper scripts for formatting, linting, and vcpkg lifecycle
- **doc/**: Project documentation including PRD (requirements document for v2.0)
- **build/**: Generated build artifacts (never committed)
- **vcpkg/cache/**: Local binary cache for vcpkg artifacts (not committed)
- **vcpkg/triplets/**: Overlay triplets for custom toolchain configuration
- **Custom triplets**: Uses `vcpkg/triplets/arm64-linux-gnu.cmake` with GNU toolchain overlay

## Critical Workflows

### Building & Testing

```bash
# Configure (select preset: lint, debug, release)
cmake --preset debug

# Build
cmake --build --preset debug-build

# Test (when tests exist)
ctest --preset debug-test --output-on-failure

# Clean rebuild
cmake --build --preset debug-rebuild

# Package
cpack --preset release-package

# Full workflow (configure + build + test + package)
cmake --workflow --preset debug-workflow
```

Use VS Code tasks for convenience: "cmake: build all", "cmake: test", "cmake: configure"

**Available CMake Presets:**
- **Configure**: `lint`, `debug`, `release`
- **Build**: `lint-build`, `lint-rebuild`, `debug-build`, `debug-rebuild`, `release-build`, `release-rebuild`
- **Test**: `debug-test`, `release-test`
- **Package**: `debug-package`, `release-package` (generates TGZ archives)
- **Workflow**: `debug-workflow`, `release-workflow` (runs full pipeline)

### VS Code (Current)

- Tasks live in `.vscode/tasks.json` and are driven by the active CMake preset (CMake Tools).
- Debugging lives in `.vscode/launch.json` and resolves the executable via CMake Tools (launch target path).
- Future/required VS Code integration behaviour is specified in `doc/PRD.md` section 7.3.

### Dependency Management

- vcpkg is tracked as a git submodule at `external/vcpkg/`
- Add dependencies to `vcpkg.json` (manifest file)
- Use `find_package(<Pkg> CONFIG REQUIRED)` in CMakeLists.txt
- Link via imported targets: `target_link_libraries(... PRIVATE <Pkg>::<Pkg>)`
- Submodule initialisation: `git submodule update --init --recursive`
- vcpkg bootstrap: `./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics`
- Or use convenience script: `./script/vcpkg-bootstrap.sh`

### Architecture Selection

The template provides architecture-specific CMake presets:
- **arm64-linux-gnu**: Custom triplet for ARM64 systems with GNU toolchain (primary supported platform)
- **x64-linux**: Standard preset for 64-bit Intel/AMD systems (untested, planned support)
- Default presets (`debug`, `release`) use ARM64
- Select architecture explicitly when needed: `cmake --preset debug-arm64` or `cmake --preset debug-x64`

### Code Quality Tools

```bash
# Format all C++ files (uses .clang-format with Mozilla style)
./script/clang-format-all.sh

# Lint all C++ files (uses .clang-tidy configuration)
./script/clang-tidy-all.sh
```

Use `lint` preset to enable clang-tidy during build.

## Project-Specific Conventions

### Naming (LLVM/Clang-aligned with PascalCase filenames)

- **Files defining types**: PascalCase (e.g., `MappedFile.hpp`, `MappedFile.cpp`)
- **Utility/test files**: PascalCase with descriptive suffix (e.g., `LoggerFactoryTest.cpp`, `AppIntegrationTest.cpp`)
- **Types**: PascalCase (e.g., `MappedFile`, `ExitCode`)
- **Functions**: camelCase (e.g., `openReadonly()`, `printUsage()`)
- **Variables**: camelCase (e.g., `filePath`, `bufferSize`)
- **Constants**: PascalCase or ALL_CAPS
- **Namespaces**: camelCase (e.g., `myProj`, `testHelpers`)
- **Test executables**: PascalCase with standardised suffixes:
  - Unit tests: `<lib-name>UnitTest` (e.g., `loggerUnitTest`, `dataProcessorUnitTest`)
  - Integration tests: `<component-name>IntTest` (e.g., `appIntTest`, `databaseIntTest`)
- **Test suites (GTest fixtures)**: PascalCase with descriptive suffix:
  - Unit tests: `<class-name>Test` (e.g., `LoggerFactoryTest`, `ConsoleLoggerTest`)
  - Integration tests: Descriptive name ending in `Suite` using `Int` abbreviation (e.g., `AppLoggingIntSuite`, `AppLifecycleSuite`)

### C++ Standards

- **C++23** is the default language standard
- Use modern idioms: RAII, smart pointers, `std::format`, concepts, ranges
- Enable strict warnings: `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion`
- Treat warnings as errors (enabled by default in target compile options)

### CMake Patterns

- **Minimum version**: CMake 3.28.3 (enforced at root)
- **Target-based architecture**: Model all build artefacts as targets, express all requirements via target commands
- **Scope discipline**: Use correct CMake target scopes:
  - `PRIVATE`: Implementation-only (compile options, private deps, internal includes)
  - `PUBLIC`: Required by target and consumers (transitive deps, public API)
  - `INTERFACE`: Required only by consumers (header-only libs, usage requirements)
- **Never** mutate global state (`include_directories()`, `link_libraries()`, `add_definitions()`, global `CMAKE_CXX_FLAGS`)
- Express language standard via `target_compile_features(... PRIVATE cxx_std_23)`, not global `CMAKE_CXX_STANDARD`
- Keep CMakeLists.txt structure uniform across all modules
- Prefer CMake Presets over cache variables for developer options

**See PRD section 5.1.6 for complete target-based architecture requirements.**

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

**See PRD section 5.4 and ARCHITECTURE.md "Adding Tests" for complete testing requirements and examples.**

### Build Hygiene

- Out-of-source builds enforced (root CMakeLists.txt checks and errors)
- Sanitizers (ASAN, UBSAN, TSAN) available via CMake options but **only** in debug builds
- `CMAKE_EXPORT_COMPILE_COMMANDS=ON` for IDE/tool integration (compile_commands.json)
- vcpkg binary caching via `VCPKG_BINARY_SOURCES="clear;files,${sourceDir}/vcpkg/cache,readwrite"`
- vcpkg metrics disabled: `VCPKG_BOOTSTRAP_OPTIONS="-disableMetrics"`
- vcpkg install options: `VCPKG_INSTALL_OPTIONS="--clean-after-build"` (keeps builds lean)

## Future Architecture (PRD Reference)

**When implemented, this template will support:**
- Modular library structure with public/private API separation (PRD section 6.2)
- Interface+factory pattern for encapsulation (PRD section 6.3, with CMake example)
- GoogleTest-based unit and integration testing (PRD section 5.4)
- Composition roots that wire dependencies only in `app/*/src/main.cpp` (PRD section 6.2)
- DevContainer setup and CI/CD workflow (PRD sections 5.3, 8)
- Project rename script (PRD section 7.1)

**Consult `doc/PRD.md` for:**
- Complete naming conventions (section 6.1)
- CMake target scope requirements (section 5.1.6)
- Testing strategy and GoogleTest integration (section 5.4)
- Architectural patterns and encapsulation rules (section 6)
- CI/CD pipeline requirements (section 8)

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
- ❌ Don't use global CMake mutations - see PRD section 5.1.6 for target-based requirements
- ❌ Don't set `CMAKE_CXX_STANDARD` globally - use `target_compile_features(... PRIVATE cxx_std_23)`
- ❌ Don't commit `build/`, `vcpkg/cache/`, or `CMakeUserPresets.json`

- ❌ Don't forget to initialise submodules after cloning (`git submodule update --init --recursive`)
- ❌ Don't use untested x64 presets without verifying on x64 hardware
- ❌ Don't move vcpkg overlay triplets out of `vcpkg/triplets/` (breaks preset paths)

**For future architecture pitfalls (interface+factory, composition roots), see PRD section 6.**

## Quick Reference

| Task | Command |
|------|---------|
| Bootstrap vcpkg | `./script/vcpkg-bootstrap.sh` |
| Configure debug | `cmake --preset debug` |
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
- Consult `doc/PRD.md` for requirements on unimplemented features
- Use Australian English spelling and grammar
- Comment code thoroughly with Doxygen-compatible documentation
- You are working in a DevContainer, ensure that the configuration is maintained
- Prefer explicitness and clarity over brevity in code, configurations, and documentation
