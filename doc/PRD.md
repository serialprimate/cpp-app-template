# Product Requirements Document: cpp-app-template (Next Version Template Repository)

| **Project Name** | cpp-app-template |
| :--- | :--- |
| **Version** | 2.0.0 (Draft) |
| **Status** | Draft for Review |
| **Type** | GitHub Template Repository |
| **Primary Language Standard** | C++23 |
| **Target Audience** | C++ Developers, Systems Engineers, DevOps Engineers |

---

## 1. Executive Summary

**cpp-app-template** provides a modern, opinionated, production-ready starting point for modular C++ applications. It is designed to remove the “configuration tax” by shipping:

- A consistent **project layout** for apps, libraries, and tests.
- A **reproducible build** setup using **CMake Presets** and **vcpkg manifest mode**.
- A “golden path” **DevContainer** experience for VS Code.
- Built-in quality tooling: **clang-format**, **clang-tidy**, and a test harness.
- Practical build hygiene that avoids common CMake pitfalls (e.g., enforcing out-of-source builds and discouraging ambient system package discovery).
- Optional packaging via **CPack** for producing distributable artefacts.

**Current Implementation Status**: The repository currently implements a minimal working template with:
- Single application structure (`app/sampleApp/`)
- vcpkg manifest mode with pinned baseline
- CMake presets (debug, debug-clang-tidy, release) with workflow support
- Build hygiene (out-of-source enforcement, sanitiser guards)

This PRD specifies requirements for the complete v2.0 template. See section 9 (Implementation Roadmap) for phased delivery.

---

## 2. Goals, Non-Goals, and Assumptions

### 2.1 Goals

1. Provide an unambiguous and contemporary specification for a next-version template repository.
2. Preserve the **modular C++ structure** and overall layout proposed in this PRD.
3. Make the build and dependency story explicit for **local development** and **CI/CD**.
4. Ensure examples are internally consistent with the directory structure and architecture rules.

### 2.2 Non-Goals

1. Implementing the repo restructure.
2. Porting existing code or scripts.
3. Selecting or enforcing a particular IDE beyond providing first-class VS Code support.
4. Supporting non-Linux platforms.

### 2.3 Assumptions

- The template targets **Linux only**. Support for other platforms is not provided or planned.
- The repo aims to be “batteries included” for build, lint, and test, but still easy to delete/replace.

---

## 3. Core Design Principles

1. **Opinionated but extensible:** Provide a default stack and defaults that work out-of-the-box while keeping configuration in standard, discoverable files.
2. **Reproducibility first:** Prefer deterministic builds and pinned dependency baselines.
3. **Modern standards:** Default to C++23 and modern CMake patterns.
4. **Testability by default:** Encourage separable modules with clean seams for unit and integration testing.
5. **Encapsulation:** Keep public interfaces stable and implementation details private by default.

---

## 4. Proposed Directory Structure & Layout

The project follows a modified pitchfork layout optimised for modular application development.

```text
/
├── .devcontainer/              # VS Code DevContainer configuration
│   ├── devcontainer.json       # Env vars, extensions, mounts
│   └── Dockerfile              # Base image definition
├── .github/
│   └── workflows/              # CI/CD pipelines
├── app/                        # Application Entry Points
│   └── <app-name>/
│       ├── CMakeLists.txt
│       └── src/
│           └── main.cpp        # Composition Root
├── lib/                        # Modular Libraries
│   └── <lib-name>/
│       ├── CMakeLists.txt
│       ├── include/
│       │   └── <lib-name>/     # Public headers (interfaces and selected public implementations)
│       └── src/                # Private implementation
├── test/
│   ├── unit/                   # Unit Tests (mirrors lib structure)
│   │   └── <lib-name>UnitTest/
│   │       ├── CMakeLists.txt
│   │       └── src/
│   └── int/                    # Integration tests (abbreviated from 'integration')
│       └── <component-name>IntTest/
│           ├── CMakeLists.txt
│           └── src/
├── doc/                        # Documentation
├── script/                     # Helper scripts (rename, bootstrap, lint)
├── vcpkg.json                  # Dependency manifest (manifest mode)
├── vcpkg-configuration.json    # Registry + baseline pinning
├── vcpkg-triplets/             # Overlay triplets (optional but supported)
├── CMakeLists.txt              # Root build orchestrator
├── CMakePresets.json           # Configure/build/test/package presets
├── CMakeUserPresets.json       # Optional local developer overrides (NOT required in CI)
├── .clang-tidy                 # clang-tidy configuration
├── .clang-format               # clang-format configuration
└── .vscode/                    # VS Code settings (optional)
```

**Constraint:** The overall modular structure above must remain intact. Additions may introduce new files/directories, but must not change the core layout.

---

## 5. Functional Requirements

### 5.1 Build System (CMake)

1. **CMake version:** Must require CMake **3.28+**.
2. **Out-of-source builds:** Must hard-fail when configuring inside any directory containing a `CMakeLists.txt` (prevents accidental in-source builds).
3. **Presets:** Must provide `CMakePresets.json` using schema `"version": 8`.
   - Must provide configure presets: `debug`, `debug-clang-tidy`, `release` (minimum)
   - Must provide build presets with rebuild variants (e.g., `debug-rebuild`)
   - Must provide test presets for debug and release configurations
   - Must provide package presets for generating distributable artefacts
   - Must provide workflow presets that chain configure → build → test → package
   - Base presets must be marked `"hidden": true` to avoid accidental direct use
4. **Generators:**
   - Must support a fast single-config generator (prefer **Unix Makefiles** initially).
5. **C++ standard:** Must set C++23, require the standard, and disable compiler extensions.
6. **Target-based architecture (critical requirement):**
   - Must model all build artefacts as CMake targets (`add_executable()`, `add_library()`).
   - Must express all requirements via target commands with explicit scopes:
     - `PRIVATE`: Required to build the target itself (implementation-only dependencies, compile options, include paths)
     - `PUBLIC`: Required by both the target and its consumers (transitive dependencies, public API requirements)
     - `INTERFACE`: Required only by consumers, not by the target itself (header-only libs, usage requirements)
   - Must never use global mutations: `include_directories()`, `link_libraries()`, `add_definitions()`, global `CMAKE_CXX_FLAGS`, etc.
   - Must express language standard requirements via `target_compile_features(... PRIVATE cxx_std_23)` not global `CMAKE_CXX_STANDARD`.
   - Must use `target_sources()` with `FILE_SET HEADERS` for public headers intended for installation.
7. **Hermetic-ish find behaviour (preferred current behaviour):**
   - Must disable usage of CMake and system environment package registries.
   - Must avoid “ambient” dependency resolution where practical.
7. **Sanitisers (preferred current behaviour):**
   - Must be controlled via CMake options and/or presets (not always-on).
   - Must ensure sanitisers are not enabled for release packaging presets.
8. **Compile database:** Must set `CMAKE_EXPORT_COMPILE_COMMANDS=ON` for developer presets.
9. **Link-time optimisation:** Release presets must use CMake’s IPO (`CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`) rather than hard-coding toolchain flags.
10. **Packaging:** Must support CPack packaging presets for at least one archive format (e.g., TGZ).
11. **Warning discipline:** Must enable strict warnings (`-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion` for GCC/Clang) and treat warnings as errors.

### 5.2 Dependency Management (vcpkg)

1. **Mode:** Must use **vcpkg manifest mode** with `vcpkg.json` and `vcpkg-configuration.json`.
2. **Integration:** Must use vcpkg as a git submodule at `./vcpkg` (current implementation).
3. **Pinning:** Must pin dependency resolution via `vcpkg-configuration.json` (registry + baseline). This pin must be treated as authoritative for both dev and CI.
4. **Triplets:**
   - Must support selecting a `VCPKG_TARGET_TRIPLET` via presets.
   - Must support overlay triplets via `VCPKG_OVERLAY_TRIPLETS` (current: `vcpkg-triplets/arm64-linux-gnu.cmake`).
   - Overlay triplets may chain to additional toolchain files (current: `toolchain-gnu.cmake`).
5. **Metrics:** Must disable vcpkg metrics in CI and DevContainer flows via `VCPKG_BOOTSTRAP_OPTIONS="-disableMetrics"`.
6. **Binary caching (preferred current behaviour):**
   - Must support repo-local binary caching using `VCPKG_BINARY_SOURCES`, with a default path of `./vcpkg-cache`.
   - Format: `VCPKG_BINARY_SOURCES="clear;files,${sourceDir}/vcpkg-cache,readwrite"`.
   - May also support `VCPKG_DEFAULT_BINARY_CACHE`, but the documented "golden path" must use `VCPKG_BINARY_SOURCES` because it is explicit and works consistently in CI.
7. **Install hygiene:** Must document (and may enable) `VCPKG_INSTALL_OPTIONS="--clean-after-build"` for CI to keep runners lean.

### 5.3 Development Environment (DevContainer)

1. **Base image:** Must use Ubuntu 24.04 LTS (or later LTS, if updated).
2. **Toolchain:** Must include a modern GCC for builds and LLVM tooling (clang-tidy/clang-format). Exact versions are implementation detail, but must be “current stable” for the base image.
3. **VS Code extensions:** Must recommend C/C++ and CMake tooling extensions.
4. **Bootstrap:** Must run a post-create bootstrap that ensures vcpkg is available and bootstrapped at the pinned baseline.
5. **Caching:** Must configure binary caching so that container rebuilds do not require rebuilding third-party dependencies.

### 5.4 Testing

1. **Framework:** Must support GoogleTest for unit and integration tests.
2. **Structure:**
   - Unit tests must be organised in folders matching their target names (`test/unit/<target-name>/` e.g., `test/unit/loggerUnitTest/`).
   - Integration tests must be organised in folders matching their target names (`test/int/<target-name>/` e.g., `test/int/appIntTest/`).
3. **Test executable naming conventions:**
   - **Unit test executables:** Must use the convention `<lib-name>UnitTest` (e.g., `loggerUnitTest`, `dataProcessorUnitTest`).
   - **Integration test executables:** Must use the convention `<component-name>IntTest` (e.g., `appIntTest`, `databaseIntTest`).
4. **Test structure conventions:**
   - **Unit tests:**
     - Each test suite is named after the class being tested (e.g., `LoggerFactoryTest` in file `LoggerFactoryTest.cpp`).
     - Each test case tests an individual method or behaviour of the class.
     - Use GTest's value-parameterised tests (`TEST_P`, `INSTANTIATE_TEST_SUITE_P`) when a single method requires testing with multiple input values.
   - **Integration tests:**
     - Each test suite represents a collection of related use-cases or scenarios.
     - Each test case represents an individual scenario (e.g., "ApplicationStartupSequence", "HighVolumeLoggingScenario").
     - Test suites should use descriptive names ending in "Suite" with "Int" abbreviation (e.g., `AppLoggingIntSuite`, `AppLifecycleSuite`).
5. **Linking rules:**
   - Unit tests must link only the library-under-test and its declared public dependencies.
   - Integration tests may link multiple internal libraries.
6. **Test execution:**
   - Must use CTest with presets (`debug-test`, `release-test`).
   - Must configure `outputOnFailure: true` for test presets.
   - Must configure `stopOnFailure: true` to fail fast.
   - Must configure `noTestsAction: error` to catch missing test registration.
7. **CMake integration:**
   - Must call `enable_testing()` in root CMakeLists.txt when `BUILD_TESTING` is ON.
   - Must register tests via `add_test()` or GoogleTest's `gtest_discover_tests()`.

### 5.5 Static Analysis and Formatting

1. **clang-format:** Must provide a repository-wide `.clang-format`.
2. **clang-tidy:** Must provide a `.clang-tidy` and a dedicated "lint" workflow/preset.
3. **Headers from dependencies:** Must configure linting to avoid noisy diagnostics from third-party headers.
4. **Script tooling:** Must provide convenience scripts (`script/clang-format-all.sh`, `script/clang-tidy-all.sh`).

### 5.6 Language-Specific AI Coding Instructions

Must provide language-specific instruction files for AI coding assistants:

1. **C++ instructions**: `.github/instructions/cpp.instructions.md` - C++23 patterns, memory management, type safety, naming conventions.
2. **CMake instructions**: `.github/instructions/cmake.instructions.md` - Modern CMake practices, target-based builds, preset usage.
3. **Bash instructions**: `.github/instructions/bash.instructions.md` - Strict mode, quoting, modern idioms.
4. **Python instructions**: `.github/instructions/python.instructions.md` - Type hints, Python 3.12+ features.

**Rationale**: Ensures consistent code generation and modification by AI tools across the development workflow.

---

## 6. Architectural Standards

The template promotes dependency injection (constructor injection by default) and clear module boundaries.

### 6.1 Naming Conventions

The project follows LLVM/Clang-aligned naming with PascalCase filenames for type definitions:

- **Files defining types**: PascalCase (e.g., `MappedFile.hpp`, `MappedFile.cpp`)
- **Utility/test files**: PascalCase with descriptive suffix (e.g., `LoggerFactoryTest.cpp`, `AppIntTest.cpp`)
- **Types (classes/structs/enums)**: PascalCase (e.g., `MappedFile`, `ExitCode`)
- **Functions**: camelCase (e.g., `openReadonly()`, `printUsage()`)
- **Variables**: camelCase (e.g., `filePath`, `bufferSize`)
- **Constants**: PascalCase or ALL_CAPS (e.g., `MaxBufferSize`, `DEFAULT_TIMEOUT`)
- **Macros**: ALL_CAPS with underscores (e.g., `MYPROJ_TEST_CASE`)
- **Namespaces**: camelCase (e.g., `myProj`, `testHelpers`)
- **Test executables**: PascalCase with standardised suffixes:
  - Unit tests: `<lib-name>UnitTest` (e.g., `loggerUnitTest`, `dataProcessorUnitTest`)
  - Integration tests: `<component-name>IntTest` (e.g., `appIntTest`, `databaseIntTest`)
- **Test suites (GTest fixtures)**: PascalCase with descriptive suffix:
  - Unit tests: `<class-name>Test` (e.g., `LoggerFactoryTest`, `ConsoleLoggerTest`)
  - Integration tests: Descriptive name ending in `Suite` using `Int` abbreviation (e.g., `AppLoggingIntSuite`, `AppLifecycleSuite`)

**Rationale**: Consistency with LLVM project conventions while maintaining clear visual distinction between types and utilities. Test naming follows a standardised pattern that clearly identifies the type of test and what is being tested.

### 6.2 Public vs Private API Rules

1. **Public headers:** Must live under `lib/<lib-name>/include/<lib-name>/...`.
2. **Private implementation:** Must live under `lib/<lib-name>/src/...`.
3. **Interfaces:** Should be public.
4. **Implementations:**
   - Should be private by default.
   - May be public only when the implementation is intentionally part of the public “wiring surface” of a module.
5. **Composition root:** Only `app/<app-name>/src/main.cpp` may wire concrete implementations together.

### 6.3 Example: Interface + Factory Pattern (Consistent Encapsulation)

This example avoids exposing a concrete implementation header to the application by providing a factory function.

**CMake structure for this pattern:**

```cmake
# lib/logger/CMakeLists.txt
add_library(logger)

target_sources(logger
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/logger/ILogger.hpp
        include/logger/LoggerFactory.hpp
    PRIVATE
        src/ConsoleLogger.cpp
)

target_include_directories(logger
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>
)

target_compile_features(logger PRIVATE cxx_std_23)
```

**Code example:**

```cpp
// lib/logger/include/logger/ilogger.h
#pragma once

#include <string_view>

namespace myapp::logger {

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(std::string_view message) = 0;
};

} // namespace myapp::logger
```

```cpp
// lib/logger/include/logger/loggerFactory.h
#pragma once

#include <logger/ilogger.h>

#include <memory>

namespace myapp::logger {

[[nodiscard]] std::unique_ptr<ILogger> createDefaultLogger();

} // namespace myapp::logger
```

```cpp
// lib/logger/src/consoleLogger.cpp

#include <logger/loggerFactory.h>

#include <iostream>

namespace myapp::logger {

namespace {

class ConsoleLogger final : public ILogger {
public:
    void log(std::string_view message) override
    {
        std::cout << message << '\n';
    }
};

} // namespace

std::unique_ptr<ILogger> createDefaultLogger()
{
    auto result = std::make_unique<ConsoleLogger>();
    return result;
}

} // namespace myapp::logger
```

```cpp
// app/sampleApp/src/main.cpp

#include <logger/loggerFactory.h>

int main()
{
    auto logger = myapp::logger::createDefaultLogger();
    logger->log("Hello from cpp-app-template");

    return 0;
}
```

---

## 7. Developer Workflow (UX)

### 7.1 Initialisation

1. User selects “Use this template” on GitHub.
2. User clones the repository.
3. User runs the rename script:

```bash
python3 script/rename_project.py --name "superCam"
```

4. User opens VS Code.
5. VS Code detects `.devcontainer` and prompts to reopen inside the container.

**Rename script requirements:**

- Must rename relevant directories and CMake target names.
- Must update namespaces in sample code (where present).
- Must not rewrite third-party or generated directories (e.g., `build/`, `vcpkg-cache/`).

### 7.2 Inner Loop

1. **Configure:** Developer runs `cmake --preset debug` (or uses CMake Tools).
2. **Build:** Developer runs `cmake --build --preset debug-build`.
3. **Test:** Developer runs `ctest --preset debug-test --output-on-failure`.
4. **Lint:** Developer runs `cmake --build --preset lint` (or a dedicated lint preset) to execute clang-tidy.

### 7.3 VS Code Integration

The template must provide a VS Code configuration that supports the “golden path” workflow via CMake Presets and the VS Code CMake Tools extension.

1. **Files:** Must include:
    - `.vscode/tasks.json` for common build/test tasks.
    - `.vscode/launch.json` for debugging the active CMake target.
2. **Preset alignment:** Tasks and launch configurations must be driven by the active CMake presets (not hardcoded build directories).
    - Tasks should use CMake Tools preset variables (e.g. `${command:cmake.activeConfigurePresetName}`, `${command:cmake.activeBuildPresetName}`, `${command:cmake.activeTestPresetName}`).
    - Launch configurations should resolve the executable from CMake Tools (e.g. `${command:cmake.launchTargetPath}`).
3. **Task requirements (minimum):** Must provide tasks for:
    - Configure
    - Build (all)
    - Test
    - Lint (clang-tidy)
    - Clean and/or rebuild (optional but recommended)
4. **Debug requirements (Linux golden path):** Must provide a debug launch configuration that:
    - Uses the active CMake launch target.
    - Works in DevContainer environments (no external console assumed).
    - Uses a supported MI debugger (`gdb` for Linux).
5. **Portability constraints:**
    - Must not hardcode absolute tool paths (e.g. `/usr/bin/cmake`) in tasks.
    - Must prefer `type: "cmake"` tasks (CMake Tools integration) over shell tasks for core workflows.
6. **Scope:** Linux only. Support for other platforms is not provided or planned.

---

## 8. CI/CD Strategy (GitHub Actions)

The repo must include a Linux CI workflow that validates configure, build, test, and lint.

### 8.1 vcpkg Strategy: Local vs CI (Required Clarification)

**Local development (DevContainer and non-container):**

- `VCPKG_ROOT` points to a developer-owned vcpkg clone (for example `./vcpkg` within the workspace).
- Binary caching defaults to a repo-local directory:

```bash
export VCPKG_BINARY_SOURCES="clear;files,${PWD}/vcpkg-cache,readwrite"
```

**CI/CD:**

- CI must not depend on a pre-installed system vcpkg.
- CI must set binary caching to the same repo-local cache path and use GitHub Actions caching to persist it across runs.
- CI may bootstrap vcpkg into a temporary directory or a workspace directory, but must ensure metrics are disabled.

### 8.2 Pipeline Steps (Minimum)

1. Checkout.
2. Restore cache for `vcpkg-cache/` (keyed on OS + `vcpkg.json` + `vcpkg-configuration.json`).
3. Configure with a CI preset.
4. Build.
5. Test.
6. Run clang-tidy (either as a separate job or as a preset).
7. Save cache for `vcpkg-cache/`.

---
## 9. Implementation Roadmap (Requirements-Only)

This roadmap defines **verifiable outcomes** for delivering the v2.0 template as specified in this PRD. Each phase lists required artefacts and **acceptance criteria** (what must be true for the phase to be considered complete).

### Phase 1: MVP (“Golden Path” Template)
**Status: ✅ IMPLEMENTED (2026-01-01)**
**Objective:** A newly created repo from the template can be opened in the DevContainer and run through a full configure → build → test loop using CMake Presets, with vcpkg manifest mode providing reproducible dependencies.

**Required deliverables**

1. **Repository layout (PRD §4)**
    - Implement the directory skeleton:
      - `app/<app-name>/src/main.cpp`
      - `lib/<lib-name>/{include/<lib-name>/,src/}`
      - `test/unit/<lib-name>/src/`
      - `test/int/<use-case>/src/`
      - `doc/`, `script/`, `.devcontainer/`, `.github/workflows/` (stubs allowed if needed for CI until Phase 2/3)
    - Keep the modular structure intact (no layout substitutions such as `source/` for v2.0).

2. **CMake build system baseline (PRD §5.1)**
    - Root `CMakeLists.txt` must:
      - Require CMake **3.28+**.
      - Hard-fail in-source configuration (including configuring inside any folder containing a `CMakeLists.txt`).
      - Use a **target-based architecture** only (no global include/link/definitions mutations).
      - Enable strict warnings and treat warnings as errors (GCC/Clang).
      - Use `target_compile_features(... PRIVATE cxx_std_23)` (no global `CMAKE_CXX_STANDARD` requirement for project targets).
      - Enable `enable_testing()` when `BUILD_TESTING` is ON.
    - Module `CMakeLists.txt` files must exist for:
      - One sample app target under `app/<app-name>/`.
      - One sample library target under `lib/<lib-name>/`.
      - One sample unit test target under `test/unit/<lib-name>/`.
      - One sample integration test target under `test/int/<use-case>/`.

3. **CMake Presets baseline (PRD §5.1.3)**
    - Provide `CMakePresets.json` with schema `"version": 8`.
    - Provide at minimum:
      - Configure presets: `debug`, `debug-clang-tidy`, `release`
      - Build presets: `debug-build`, `release-build`, plus rebuild variants (for example `debug-rebuild`, `release-rebuild`)
      - Test presets: `debug-test`, `release-test`
      - Package presets: may be stubbed until Phase 2, but presets structure must exist if referenced by workflows
      - Workflow presets: may be stubbed until Phase 2, but naming and chaining design must be documented
    - Base presets must be `"hidden": true`.
    - Generator must be a supported fast single-config generator (prefer **Unix Makefiles**).

4. **vcpkg manifest + caching (PRD §5.2, §8.1)**
    - Provide `vcpkg.json` and `vcpkg-configuration.json` with a pinned baseline.
    - Template must use vcpkg as a git submodule at `./vcpkg`.
    - Document and default (via presets and/or DevContainer env) repo-local binary caching:
      - `VCPKG_BINARY_SOURCES="clear;files,${sourceDir}/vcpkg-cache,readwrite"`
    - Metrics must be disabled in the “golden path” flow:
      - `VCPKG_BOOTSTRAP_OPTIONS="-disableMetrics"`
    - If overlay triplets are referenced, they must exist and be wired via presets.

5. **Testing minimum (PRD §5.4)**
    - Add GoogleTest dependency via vcpkg and integrate it via CMake (`find_package(GTest CONFIG REQUIRED)`).
    - Provide:
      - At least one **unit test** target under `test/unit/<lib-name>/` that links only the library-under-test and its declared public deps.
      - At least one **integration test** target under `test/int/<use-case>/` that can link multiple internal libraries if required.
    - CTest presets must enforce:
      - `outputOnFailure: true`
      - `stopOnFailure: true`
      - `noTestsAction: error`

6. **DevContainer “opens and runs” (PRD §5.3)**
    - DevContainer must be based on Ubuntu 24.04 LTS.
    - Post-create bootstrap must ensure vcpkg is bootstrapped and ready to use at the pinned baseline.
    - Document the expected inner-loop commands (configure/build/test) using presets.

**Acceptance criteria**

- In a clean clone (Linux), the following must succeed without manual edits:
  - `cmake --preset debug`
  - `cmake --build --preset debug-build`
  - `ctest --preset debug-test --output-on-failure`
- The sample app compiles and runs (manual invocation acceptable; packaging not required in Phase 1).
- No in-source build is possible without an explicit failure message explaining the problem.
- Dependency resolution occurs via vcpkg manifest mode (no ambient system package discovery required to succeed).

**Notes**
- Phase 1 may use stubs or minimal implementations for scripts, packaging, and CI until Phase 2.
- Phase 1 may reuse some preexisting sample code/artifacts from the current implementation as long as they meet minimum requirements.
  It is acceptable for them to exceed the minimum requirements (where appropriate).

---

### Phase 2: Quality of Life + Packaging + CI Baseline

**Objective:** Make the template comfortable to use day-to-day, and ensure CI validates the golden path. Add packaging support for distributable artefacts.

**Required deliverables**

1. **Rename script (PRD §7.1)**
    - Implement `script/rename_project.py` with documented usage and help output.
    - Must update, at minimum:
      - App and library directory names where applicable.
      - CMake target names and references.
      - Namespaces in sample code (where present).
      - Documentation identifiers (README/PRD references as appropriate).
    - Must not rewrite third-party or generated directories (`build/`, `vcpkg-cache/`, `vcpkg/`).
    - Must be idempotent or fail safely with clear guidance.

2. **CPack packaging presets (PRD §5.1.10)**
    - Add CPack configuration sufficient to generate at least one archive format (TGZ is acceptable).
    - Provide `debug-package` and `release-package` presets (or equivalent) and ensure release packaging does not enable sanitisers.

3. **Lint integration (PRD §5.5)**
    - Provide `.clang-format` and `.clang-tidy` based on LLVM/Clang defaults with sensible project-specific adjustments.
    - Configure `.clang-tidy` to suppress diagnostics from system and third-party headers.
    - Provide a dedicated lint preset and/or build preset behaviour for `debug-clang-tidy`.
    - Provide convenience scripts under `script/` (for example `script/clang-format-all.sh`, `script/clang-tidy-all.sh`) consistent with the PRD.

4. **GitHub Actions CI (PRD §8)**
    - Add a Linux-only workflow that runs:
      1. Configure (CI preset)
      2. Build
      3. Test
      4. Lint (clang-tidy)
    - Cache `vcpkg-cache/` keyed on OS + `vcpkg.json` + `vcpkg-configuration.json`.
    - Ensure vcpkg metrics are disabled in CI.

**Acceptance criteria**

- `cmake --workflow --preset <debug-workflow>` and/or `<release-workflow>` executes end-to-end (configure/build/test/package) where defined.
- CI passes on pull requests with a cold cache and with a warmed cache.
- Packaging preset produces a TGZ artefact in release configuration.

---

### Phase 3: Optional Scaffolder (Feature Toggles)

**Objective:** Provide an optional generator for organisations wanting selectable features while preserving the template’s “works by default” behaviour.

**Required deliverables (only if implemented)**

- A generator wrapper (for example Copier) that can instantiate:
  - App name, library name defaults, namespace root, and optional features (tests, lint, packaging).
- The generated output must remain conformant to:
  - Directory structure rules (PRD §4)
  - CMake target-based rules (PRD §5.1.6)
  - Testing structure (PRD §5.4)
  - Linux-only constraint (PRD §2.2, §7.3.6)

**Acceptance criteria (only if implemented)**

- A newly generated project can run the same golden path commands as Phase 1 without manual edits.
- The generator does not introduce additional mandatory runtime dependencies for users who prefer the plain GitHub template flow.

---

## 10. Risks and Mitigations

| Risk | Impact | Mitigation |
| :--- | :--- | :--- |
| vcpkg download or registry outages | CI failures | Use binary caching; optionally mirror registries in enterprise environments |
| Docker performance (macOS/Windows) | Slow file I/O | Keep `build/` inside the container filesystem; avoid bind mounts for build output |
| Accidental ambient dependency pickup | Non-reproducible builds | Prefer hermetic find settings and vcpkg toolchain |

---

## 11. Appendix: Configuration Examples (Illustrative)

These snippets are examples; exact values may vary by platform.

### 11.1 CMakePresets.json (Local + CI, With vcpkg Cache)

```json
{
    "version": 8,
    "configurePresets": [
        {
            "name": "base-configure",
            "hidden": true,
            "generator": "Unix Makefiles",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "installDir": "${sourceDir}/build/${presetName}/install",
            "cacheVariables": {
                "CMAKE_CXX_STANDARD": "23",
                "CMAKE_CXX_STANDARD_REQUIRED": "ON",
                "CMAKE_CXX_EXTENSIONS": "OFF",
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
                "CMAKE_INTERPROCEDURAL_OPTIMIZATION": "OFF",
                "ENABLE_ASAN": "OFF",
                "ENABLE_UBSAN": "OFF",
                "ENABLE_TSAN": "OFF",
                "VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/vcpkg-triplets",
                "VCPKG_INSTALL_OPTIONS": "--clean-after-build"
            },
            "environment": {
                "VCPKG_BINARY_SOURCES": "clear;files,${sourceDir}/vcpkg-cache,readwrite",
                "VCPKG_BOOTSTRAP_OPTIONS": "-disableMetrics"
            },
            "toolchainFile": "${sourceDir}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        },
        {
            "name": "debug",
            "inherits": "base-configure",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "ENABLE_ASAN": "ON",
                "ENABLE_UBSAN": "ON"
            }
        },
        {
            "name": "release",
            "inherits": "base-configure",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_INTERPROCEDURAL_OPTIMIZATION": "ON"
            }
        },
        {
            "name": "ci-linux",
            "inherits": "release",
            "cacheVariables": {
                "CMAKE_COLOR_DIAGNOSTICS": "OFF"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "debug-build",
            "configurePreset": "debug",
            "verbose": true
        },
        {
            "name": "release-build",
            "configurePreset": "release",
            "verbose": false
        }
    ],
    "testPresets": [
        {
            "name": "debug-test",
            "configurePreset": "debug",
            "output": { "outputOnFailure": true }
        }
    ]
}
```

### 11.2 DevContainer (Environment Consistency)

```json
{
    "name": "cpp-app-template",
    "containerEnv": {
        "VCPKG_ROOT": "/workspaces/${localWorkspaceFolderBasename}/vcpkg",
        "VCPKG_BINARY_SOURCES": "clear;files,/workspaces/${localWorkspaceFolderBasename}/vcpkg-cache,readwrite"
    }
}
```

### 11.3 GitHub Actions (vcpkg-cache/ Caching Sketch)

```yaml
name: ci

on:
  push:
  pull_request:

jobs:
  linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Cache vcpkg binaries
        uses: actions/cache@v4
        with:
          path: vcpkg-cache
          key: ${{ runner.os }}-vcpkg-${{ hashFiles('vcpkg.json', 'vcpkg-configuration.json') }}

      - name: Configure
        run: cmake --preset ci-linux

      - name: Build
        run: cmake --build --preset release-build

      - name: Test
        run: ctest --preset debug-test --output-on-failure
```
