# Implementation Plan: vcpkg Submodule Integration

> **✅ STATUS: COMPLETED** - This implementation plan has been fully executed. This document serves as a historical record of the vcpkg submodule integration work completed on 2026-01-02. The repository now uses vcpkg as a git submodule at `external/vcpkg/`.

| **Project** | cpp-app-template |
| :--- | :--- |
| **Phase** | Phase 1 Completion |
| **PRD Version** | 2.0.0 (Draft) |
| **Created** | 2026-01-02 |
| **Completed** | 2026-01-02 |
| **Status** | COMPLETED - Historical Record |

---

## 1. Technical Overview

This implementation plan addresses a critical gap in Phase 1: the vcpkg dependency management system is currently not tracked as a git submodule, preventing fresh clones from building successfully. The PRD explicitly requires vcpkg to be integrated as a git submodule at `./vcpkg` (PRD §5.2.2), but the current implementation relies on an ignored local clone that is not present in the repository.

**Current state:**
- `vcpkg/` directory exists locally but is ignored by `.gitignore`
- No `.gitmodules` file exists
- `utility/` contains manual setup scripts (`vcpkg-setup.sh`, `vcpkg-teardown.sh`) that are no longer needed
- CMake presets hardcode `arm64-linux-gnu` triplet (current development platform)
- Custom triplets and cache are at repository root, not organized systematically
- DevContainer `postCreateCommand` attempts to bootstrap vcpkg but will fail on fresh clones

**Target state:**
- vcpkg tracked as a git submodule at `external/vcpkg/`
- Project-owned vcpkg files organized in top-level `vcpkg/` directory:
  - `vcpkg/cache/` for binary cache
  - `vcpkg/triplets/` for custom overlay triplets
- `external/` reserved exclusively for third-party submodules
- ARM64 as primary supported architecture (x64 support planned but untested)
- Updated DevContainer with submodule initialization
- Obsolete utility scripts removed
- Comprehensive documentation for submodule workflow
- CI/CD verification of submodule presence and bootstrap

**Rationale for submodule approach:**
- Git submodules pin vcpkg to a specific commit, ensuring reproducible builds
- The vcpkg repository contains both tooling and ports/recipes; version consistency is critical
- Submodules integrate with standard git workflows (clone, update, commit)
- The `builtin-baseline` in `vcpkg.json` pins dependency versions but not the vcpkg tool itself

**Rationale for directory organization:**
- `external/` reserved exclusively for git submodules (third-party dependencies)
- `vcpkg/` at repository root for project-owned vcpkg configuration:
  - `vcpkg/cache/` - generated binary cache (can be regenerated, excluded from git)
  - `vcpkg/triplets/` - project-specific overlay triplets (version controlled)
- Clear separation between third-party code (submodules) and project configuration
- Avoids accidental deletion (cache not in `build/` folder)
- Follows principle of least surprise: submodules in `external/`, project config at root level

---

## 2. Proposed Changes

### 2.1 Git Submodule Configuration

**Files to create:**

#### `.gitmodules`
```ini
[submodule "external/vcpkg"]
	path = external/vcpkg
	url = https://github.com/microsoft/vcpkg.git
	branch = master
```

**Rationale:** Tracks vcpkg as a submodule at the recommended `external/` location. Uses `master` branch as the stable reference point (specific commit will be pinned).

---

### 2.2 `.gitignore` Updates

**File:** `.gitignore`

**Current content:**
```
/build
/vcpkg-cache
/vcpkg
```

**New content:**
```
/build
/vcpkg/cache
```

**Changes:**
- Remove `/vcpkg` entry (directory now contains project-owned files)
- Remove `/vcpkg-cache` entry (cache moved to `vcpkg/cache/`)
- Add `/vcpkg/cache` to ignore generated binary cache
- Keep `external/` clean (tracked submodules only)

---

### 2.3 CMake Presets Updates

**File:** `CMakePresets.json`

**Changes required:**

1. **Update toolchain file path:**
   - From: `"toolchainFile": "${sourceDir}/vcpkg/scripts/buildsystems/vcpkg.cmake"`
   - To: `"toolchainFile": "${sourceDir}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"`

2. **Update overlay triplets path:**
   - From: `"VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/vcpkg-triplets"`
   - To: `"VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/vcpkg/triplets"`

3. **Update binary cache path:**
   - From: `"VCPKG_BINARY_SOURCES": "clear;files,${sourceDir}/vcpkg-cache,readwrite"`
   - To: `"VCPKG_BINARY_SOURCES": "clear;files,${sourceDir}/vcpkg/cache,readwrite"`

4. **Set ARM64 as primary architecture:**
   - Current development and testing is on ARM64 platform only
   - x64 support is designed but untested (requires x64 development environment)
   - Default presets use `arm64-linux-gnu` triplet
   - x64 presets are provided but documented as untested

**Example implementation for platform detection:**

```json
{
    "name": "base-configure",
    "hidden": true,
    "generator": "Unix Makefiles",
    "binaryDir": "${sourceDir}/build/${presetName}",
    "installDir": "${sourceDir}/build/${presetName}/install",
    "cacheVariables": {
        "CMAKE_CXX_COMPILER": "g++",
        "CMAKE_INTERPROCEDURAL_OPTIMIZATION": false,
        "CMAKE_EXPORT_COMPILE_COMMANDS": true,
        "CMAKE_COLOR_DIAGNOSTICS": false,
        "BUILD_SHARED_LIBS": false,
        "ENABLE_ASAN": false,
        "ENABLE_TSAN": false,
        "ENABLE_UBSAN": false,
        "VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/vcpkg/triplets",
        "VCPKG_BOOTSTRAP_OPTIONS": "-disableMetrics",
        "VCPKG_INSTALL_OPTIONS": "--clean-after-build"
    },
    "condition": {
        "type": "equals",
        "lhs": "${hostSystemName}",
        "rhs": "Linux"
    },
    "warnings": {
        "dev": false,
        "deprecated": true,
        "uninitialized": true,
        "unusedCli": true
    },
    "errors": {
        "dev": false,
        "deprecated": true
    },
    "environment": {
        "VCPKG_BINARY_SOURCES": "clear;files,${sourceDir}/vcpkg/cache,readwrite"
    },
    "toolchainFile": "${sourceDir}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
}
```

**Add architecture-specific configure presets:**

```json
{
    "name": "debug-arm64",
    "inherits": "base-configure",
    "hidden": false,
    "displayName": "Debug (ARM64)",
    "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "VCPKG_TARGET_TRIPLET": "arm64-linux-gnu"
    }
},
{
    "name": "debug-x64",
    "inherits": "base-configure",
    "hidden": false,
    "displayName": "Debug (x64 - UNTESTED)",
    "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "VCPKG_TARGET_TRIPLET": "x64-linux"
    }
},
{
    "name": "debug",
    "inherits": "debug-arm64",
    "hidden": false,
    "displayName": "Debug (Default: ARM64)"
}
```

**Note:** The default `debug` preset inherits from `debug-arm64` as ARM64 is the currently supported and tested platform. x64 presets are provided for future use but are untested.

**Corresponding build presets:**

```json
{
    "name": "debug-x64-build",
    "inherits": "base-build",
    "configurePreset": "debug-x64",
    "verbose": true
},
{
    "name": "debug-arm64-build",
    "inherits": "base-build",
    "configurePreset": "debug-arm64",
    "verbose": true
}
```

**Update existing presets:**
- Replicate the architecture-aware pattern for `release` and `debug-clang-tidy` presets
- Update corresponding build, test, and package presets to reference the new configure presets

---

### 2.4 Custom Triplet Directory Relocation

**Action:** Move `vcpkg-triplets/` → `vcpkg/triplets/`

**Files to move:**
- `vcpkg-triplets/arm64-linux-gnu.cmake` → `vcpkg/triplets/arm64-linux-gnu.cmake`
- `vcpkg-triplets/toolchain-gnu.cmake` → `vcpkg/triplets/toolchain-gnu.cmake`

**Rationale:**
- Keeps all project-owned vcpkg configuration together in `vcpkg/` directory
- Separates project configuration from third-party submodules (`external/`)
- Custom triplets are part of the project's build configuration, not external dependencies

**File updates:**

**`vcpkg/triplets/arm64-linux-gnu.cmake`:**
No content changes required (already uses `CMAKE_CURRENT_LIST_DIR` for relative paths).

**`vcpkg/triplets/toolchain-gnu.cmake`:**
No content changes required (uses standard CMake variables).

---

### 2.5 DevContainer Configuration Updates

**File:** `.devcontainer/devcontainer.json`

**Current `postCreateCommand`:**
```json
"postCreateCommand": "bash -c 'if [ ! -f ./vcpkg/vcpkg ]; then ./vcpkg/bootstrap-vcpkg.sh -disableMetrics || exit 1; fi'"
```

**New `postCreateCommand`:**
```json
"postCreateCommand": "bash -c 'git submodule update --init --recursive && if [ ! -f ./external/vcpkg/vcpkg ]; then ./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics || exit 1; fi'"
```

**Current `containerEnv`:**
```json
"containerEnv": {
    "VCPKG_ROOT": "${containerWorkspaceFolder}/vcpkg",
    "VCPKG_BINARY_SOURCES": "clear;files,${containerWorkspaceFolder}/vcpkg-cache,readwrite"
}
```

**New `containerEnv`:**
```json
"containerEnv": {
    "VCPKG_ROOT": "${containerWorkspaceFolder}/external/vcpkg",
    "VCPKG_BINARY_SOURCES": "clear;files,${containerWorkspaceFolder}/vcpkg/cache,readwrite"
}
```

**Rationale:**
- Ensures submodule is initialized before attempting bootstrap
- Updates all path references to new directory organization
- Maintains existing bootstrap logic (only runs if vcpkg executable is missing)
    "VCPKG_ROOT": "${containerWorkspaceFolder}/external/vcpkg",
    "VCPKG_BINARY_SOURCES": "clear;files,${containerWorkspaceFolder}/external/vcpkg-cache,readwrite"
}
```

**Rationale:**
- Ensures submodule is initialized before attempting bootstrap
- Updates all path references to new `external/` location
- Maintains existing bootstrap logic (only runs if vcpkg executable is missing)

---

### 2.6 Script Updates and Removal

**Files to remove:**
- `utility/vcpkg-setup.sh` (obsolete: submodule replaces manual cloning)
- `utility/vcpkg-teardown.sh` (obsolete: submodule management via git)

**Files to keep:**
- `utility/bash-rc.sh` (still useful for shell environment setup)

**Files to move:**
- `utility/clang-format-all.sh` → `script/clang-format-all.sh` (deferred to Phase 2)
- `utility/clang-tidy-all.sh` → `script/clang-tidy-all.sh` (deferred to Phase 2)

**New script to create:** `script/vcpkg-bootstrap.sh`

**Purpose:** Provides a standalone bootstrap script for manual workflows (outside DevContainer).

**Content:**
```bash
#!/usr/bin/env bash

# Enable strict mode
set -euo pipefail

# Navigate to repository root
cd "$(dirname "${BASH_SOURCE[0]}")/.."

# Ensure submodule is initialized
if [ ! -d "external/vcpkg/.git" ]; then
    echo "Initializing vcpkg submodule..."
    git submodule update --init --recursive
fi

# Bootstrap vcpkg if needed
if [ ! -f "external/vcpkg/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    ./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics
fi

# Create cache directory if needed
mkdir -p vcpkg/cache

echo "vcpkg is ready."
```

**Make executable:**
```bash
chmod +x script/vcpkg-bootstrap.sh
```

---

### 2.7 Documentation Updates

**File:** `README.md`

**Section: "Directory Structure"**

Update the vcpkg reference:
```markdown
├── external/                # Third-party dependencies (submodules only)
│   └── vcpkg/              # vcpkg submodule (dependency manager)
├── vcpkg/                  # Project vcpkg configuration
│   ├── cache/              # Binary cache (not committed)
│   └── triplets/           # Overlay triplets for custom toolchain
```

**Section: "Getting Started" → "Option 1: DevContainer (Recommended)"**

No changes required (DevContainer handles submodule initialization automatically).

**Section: "Getting Started" → "Option 2: Local Development"**

Update step 1:
```markdown
1. **Initialize vcpkg:**
   ```bash
   git submodule update --init --recursive
   ./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics
   ```

   Or use the convenience script:
   ```bash
   ./script/vcpkg-bootstrap.sh
   ```
```

**Add new section: "Working with the vcpkg Submodule"**

```markdown
## Working with the vcpkg Submodule

This project uses vcpkg as a git submodule to ensure reproducible builds. Here's what you need to know:

### Fresh Clone

When cloning this repository, initialize the submodule:

```bash
git clone <repo-url>
cd cpp-app-template
git submodule update --init --recursive
```

Or use the `--recurse-submodules` flag during clone:

```bash
git clone --recurse-submodules <repo-url>
```

### Updating vcpkg

To update vcpkg to a newer commit:

```bash
cd external/vcpkg
git checkout master
git pull
cd ../..
git add external/vcpkg
git commit -m "Update vcpkg to latest master"
```

### Submodule Status

Check the current submodule state:

```bash
git submodule status
```

### Architecture Selection

The project currently supports ARM64 Linux as the primary tested platform:

- **ARM64 (default, tested):** `cmake --preset debug` or `cmake --preset debug-arm64`
- **x64 (untested, planned):** `cmake --preset debug-x64`

**Note:** x64 presets are provided for future compatibility but have not been tested on x64 hardware. If you are developing on an x64 platform, you may encounter issues that need to be addressed.
```

**File:** `.github/copilot-instructions.md`

**Section: "Critical Workflows" → "Dependency Management"**

Update the section:
```markdown
### Dependency Management

- vcpkg is tracked as a git submodule at `external/vcpkg/`
- Add dependencies to `vcpkg.json` (manifest file)
- Use `find_package(<Pkg> CONFIG REQUIRED)` in CMakeLists.txt
- Link via imported targets: `target_link_libraries(... PRIVATE <Pkg>::<Pkg>)`
- Submodule initialization: `git submodule update --init --recursive`
- vcpkg bootstrap: `./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics`
- Or use convenience script: `./script/vcpkg-bootstrap.sh`

### Architecture Selection

The template provides architecture-specific CMake presets:
- **arm64-linux-gnu**: Custom triplet for ARM64 systems with GNU toolchain (primary supported platform)
- **x64-linux**: Standard preset for 64-bit Intel/AMD systems (untested, planned support)
- Default preset (`debug`, `release`) uses ARM64
- Select architecture explicitly when needed: `cmake --preset debug-arm64` or `cmake --preset debug-x64`
```

**Section: "Common Pitfalls to Avoid"**

Add entries:
```markdown
- ❌ Don't forget to initialize submodules after cloning (`git submodule update --init --recursive`)
- ❌ Don't use untested x64 presets without verifying on x64 hardware
- ❌ Don't commit `vcpkg/cache/` (it's ignored for a reason)
- ❌ Don't move vcpkg overlay triplets out of `vcpkg/triplets/` (breaks preset paths)
```

**File:** `doc/ARCHITECTURE.md`

Add new section: "Dependency Management Architecture"

```markdown
## Dependency Management Architecture

### vcpkg Integration

This project uses vcpkg as a git submodule to manage external dependencies. This approach ensures:

- **Reproducibility**: The exact vcpkg commit is tracked, guaranteeing consistent dependency resolution
- **Portability**: The vcpkg toolchain and ports are versioned together
- **Hermeticity**: No reliance on system-installed packages or ambient environment

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
- `external/` reserved exclusively for git submodules (third-party code)
- `vcpkg/` at root for project-owned vcpkg configuration and generated cache
- Separates concerns: third-party dependencies vs project build configuration

### Dependency Resolution Flow

1. CMake reads `vcpkg.json` (manifest file listing dependencies)
2. vcpkg resolves dependencies using the pinned baseline in `vcpkg-configuration.json`
3. Binary cache is checked (`vcpkg/cache/`) for pre-built packages
4. Missing packages are built from source using the specified triplet
5. Built packages are cached for future builds

### Architecture Support Status

**Currently Supported:**
- ARM64 Linux with GNU toolchain (tested on aarch64 systems)

**Planned (Untested):**
- x64 Linux (presets provided but require x64 hardware for validation)

### Adding New Dependencies

1. Add the package to `dependencies` array in `vcpkg.json`
2. Reconfigure: `cmake --preset debug` (or `debug-arm64`)
3. vcpkg automatically fetches and builds the dependency
4. Use `find_package(<Pkg> CONFIG REQUIRED)` in CMakeLists.txt
5. Link via `target_link_libraries(... PRIVATE <Pkg>::<Pkg>)`

### Updating vcpkg

To update the vcpkg version:

```bash
cd external/vcpkg
git checkout master
git pull origin master
cd ../..
git add external/vcpkg
git commit -m "chore: update vcpkg to <commit-hash>"
```

Test thoroughly before committing the update.
```

---

### 2.8 CI/CD Integration (Future Phase 2 Enhancement)

**File:** `.github/workflows/ci.yml` (to be created in Phase 2)

**Requirements for vcpkg submodule integration:**

```yaml
name: CI

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build-linux:
    runs-on: ubuntu-24.04

    steps:
      - name: Checkout repository
        uses: actions/checkout@v4
        with:
          submodules: recursive  # Critical: Initialize vcpkg submodule

      - name: Cache vcpkg binaries
        uses: actions/cache@v4
        with:
          path: vcpkg/cache
          key: vcpkg-${{ runner.os }}-${{ hashFiles('vcpkg.json', 'vcpkg-configuration.json') }}
          restore-keys: |
            vcpkg-${{ runner.os }}-

      - name: Bootstrap vcpkg
        run: |
          if [ ! -f external/vcpkg/vcpkg ]; then
            ./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics
          fi

      - name: Configure
        run: cmake --preset ci-linux

      - name: Build
        run: cmake --build --preset ci-linux-build

      - name: Test
        run: ctest --preset ci-linux-test --output-on-failure
```

**Key elements:**
- `submodules: recursive` in checkout action ensures vcpkg is present
- Cache action uses `external/vcpkg-cache/` path and keys on dependency manifests
- Bootstrap step checks for vcpkg executable before running (idempotent)
- Metrics disabled via `-disableMetrics` flag

---

## 3. Dependencies

### 3.1 New Dependencies

**None.** This change uses existing infrastructure (git submodules are a core git feature).

### 3.2 Modified Dependencies

**vcpkg:**
- **Current state:** Local clone, ignored by git, manually managed
- **New state:** Git submodule at `external/vcpkg/`, pinned to specific commit
- **Justification:** PRD §5.2.2 explicitly requires vcpkg as a submodule for reproducible builds

---

## 4. Data Schema/API/Interface Contracts

### 4.1 Git Submodule Contract

**`.gitmodules` schema:**
```ini
[submodule "<path>"]
    path = <relative-path-from-repo-root>
    url = <git-repository-url>
    branch = <tracking-branch>
```

**Behavior contract:**
- `git submodule update --init --recursive`: Initialize and clone the submodule
- `git submodule status`: Show current commit hash and state
- Parent repo tracks specific submodule commit, not branch HEAD
- Updating submodule requires explicit `git add` and `commit`

### 4.2 CMake Preset Architecture Contract

**Platform-aware preset hierarchy:**

```
base-configure (hidden)
├── debug-arm64 (ARM64 Linux, debug - tested)
├── debug-x64 (x64 Linux, debug - untested)
├── release-arm64 (ARM64 Linux, release - tested)
└── release-x64 (x64 Linux, release - untested)

debug (user-facing default) → inherits debug-arm64
release (user-facing default) → inherits release-arm64
```

**Contract guarantees:**
- All presets set `VCPKG_TARGET_TRIPLET` explicitly
- All presets reference `external/vcpkg/` for submodule and `vcpkg/` for project config
- Default presets (`debug`, `release`) map to ARM64 (primary supported platform)
- x64 presets provided but marked as untested (requires x64 hardware validation)
- Architecture-specific presets support future cross-compilation scenarios

### 4.3 vcpkg Binary Cache Contract

**Cache location:** `vcpkg/cache/`

**Cache source string format:**
```
clear;files,${sourceDir}/vcpkg/cache,readwrite
```

**Semantics:**
- `clear`: Discard any inherited cache sources
- `files`: Use filesystem-based cache
- `readwrite`: Cache is both read and written during builds

**Cache invalidation:**
- Triggered by changes to `vcpkg.json` (dependency list)
- Triggered by changes to `vcpkg-configuration.json` (baseline pin)
- Triggered by vcpkg submodule update (tool version change)

---

## 5. Observability & Testing

### 5.1 Validation Tests

#### Test 1: Fresh Clone Workflow (Critical)

**Objective:** Verify that a fresh clone can build without manual intervention.

**Steps:**
1. Clone repository to a temporary location:
   ```bash
   git clone <repo-url> /tmp/cpp-app-test
   cd /tmp/cpp-app-test
   ```

2. Initialize submodule:
   ```bash
   git submodule update --init --recursive
   ```

3. Verify vcpkg directory structure:
   ```bash
   test -d external/vcpkg/.git || echo "FAIL: Submodule not initialized"
   test -f external/vcpkg/bootstrap-vcpkg.sh || echo "FAIL: Bootstrap script missing"
   ```

4. Bootstrap vcpkg:
   ```bash
   ./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics
   test -f external/vcpkg/vcpkg || echo "FAIL: vcpkg executable not created"
   ```

5. Configure:
   ```bash
   cmake --preset debug
   test $? -eq 0 || echo "FAIL: Configuration failed"
   ```

6. Build:
   ```bash
   cmake --build --preset debug-build
   test $? -eq 0 || echo "FAIL: Build failed"
   ```

7. Test:
   ```bash
   ctest --preset debug-test --output-on-failure
   test $? -eq 0 || echo "FAIL: Tests failed"
   ```

8. Clean up:
   ```bash
   rm -rf /tmp/cpp-app-test
   ```

**Expected result:** All steps succeed without errors.

---

#### Test 2: DevContainer Workflow

**Objective:** Verify DevContainer bootstrap and build.

**Steps:**
1. Open repository in VS Code
2. Accept "Reopen in Container" prompt
3. Wait for DevContainer to build and initialize
4. Check terminal output for:
   - `git submodule update --init --recursive` (should run)
   - `Bootstrapping vcpkg...` (should run if vcpkg executable missing)
   - No errors during post-create command

5. Open integrated terminal and run:
   ```bash
   cmake --preset debug
   cmake --build --preset debug-build
   ctest --preset debug-test --output-on-failure
   ```

**Expected result:** All commands succeed, tests pass.

---

#### Test 3: Architecture Selection

**Objective:** Verify ARM64 preset works correctly (x64 requires x64 hardware).

**Steps (ARM64 - Primary):**
```bash
cmake --preset debug
grep -q "VCPKG_TARGET_TRIPLET.*arm64-linux-gnu" build/debug/CMakeCache.txt
cmake --build --preset debug-build
```

**Steps (ARM64 - Explicit):**
```bash
cmake --preset debug-arm64
grep -q "VCPKG_TARGET_TRIPLET.*arm64-linux-gnu" build/debug-arm64/CMakeCache.txt
cmake --build --preset debug-arm64-build
```

**Steps (x64 - Requires x64 Hardware):**
```bash
cmake --preset debug-x64
grep -q "VCPKG_TARGET_TRIPLET.*x64-linux" build/debug-x64/CMakeCache.txt
cmake --build --preset debug-x64-build
```

**Expected result:** ARM64 presets configure and build successfully. x64 preset is provided but cannot be validated without x64 hardware.

---

#### Test 4: Binary Cache Effectiveness

**Objective:** Verify binary cache reduces rebuild time.

**Steps:**
1. Clean build (cold cache):
   ```bash
   rm -rf build/debug vcpkg/cache
   time cmake --preset debug
   time cmake --build --preset debug-build
   ```
   Record build time (should include dependency compilation).

2. Clean build again (warm cache):
   ```bash
   rm -rf build/debug
   time cmake --preset debug
   time cmake --build --preset debug-build
   ```
   Record build time (should be significantly faster).

3. Verify cache contents:
   ```bash
   test -d vcpkg/cache || echo "FAIL: Cache directory not created"
   test "$(find vcpkg/cache -type f | wc -l)" -gt 0 || echo "FAIL: Cache is empty"
   ```

**Expected result:** Second build is faster; cache directory contains files.

---

#### Test 5: Submodule Update Workflow

**Objective:** Verify updating vcpkg submodule works correctly.

**Steps:**
1. Check current vcpkg commit:
   ```bash
   git submodule status
   ```

2. Update vcpkg to latest:
   ```bash
   cd external/vcpkg
   git checkout master
   git pull origin master
   VCPKG_NEW_COMMIT=$(git rev-parse HEAD)
   cd ../..
   ```

3. Verify git detects change:
   ```bash
   git status | grep -q "external/vcpkg" || echo "FAIL: Submodule change not detected"
   ```

4. Commit the update:
   ```bash
   git add external/vcpkg
   git commit -m "chore: update vcpkg to ${VCPKG_NEW_COMMIT:0:8}"
   ```

5. Verify build still works:
   ```bash
   rm -rf build/debug
   cmake --preset debug
   cmake --build --preset debug-build
   ```

**Expected result:** Submodule update is tracked by git; build succeeds.

---

#### Test 6: Obsolete Script Removal Verification

**Objective:** Confirm obsolete scripts are removed and workflows still function.

**Steps:**
1. Verify scripts do not exist:
   ```bash
   test ! -f utility/vcpkg-setup.sh || echo "FAIL: vcpkg-setup.sh still exists"
   test ! -f utility/vcpkg-teardown.sh || echo "FAIL: vcpkg-teardown.sh still exists"
   ```

2. Verify new script exists and works:
   ```bash
   test -x script/vcpkg-bootstrap.sh || echo "FAIL: vcpkg-bootstrap.sh not executable"
   ./script/vcpkg-bootstrap.sh
   test $? -eq 0 || echo "FAIL: Bootstrap script failed"
   ```

3. Verify documentation no longer references old scripts:
   ```bash
   grep -r "vcpkg-setup.sh" README.md doc/ .github/ && echo "FAIL: Old script referenced in docs"
   ```

**Expected result:** Old scripts removed, new script works, documentation updated.

---

### 5.2 Regression Tests

**Existing test suite must continue to pass:**

```bash
ctest --preset debug-test --output-on-failure
```

**Expected output:**
```
100% tests passed, 0 tests failed out of 22
```

All existing unit and integration tests must pass without modification.

---

## 6. Key Decisions and Assumptions

### 6.1 Architectural Decisions

#### Decision 1: Directory Organization (`external/` for Submodules, `vcpkg/` for Project Config)

**Options considered:**
- Keep all vcpkg-related files in `external/` (submodule, cache, triplets)
- Use `third_party/` for submodules
- Separate submodules (`external/`) from project configuration (root-level folders)

**Decision:** Use `external/` exclusively for git submodules; create `vcpkg/` at repository root for project-owned vcpkg configuration (cache, triplets).

**Rationale:**
- `external/` reserved for third-party code tracked as submodules
- `vcpkg/` contains project-specific configuration, not external dependencies
- Clear semantic separation: submodules vs project build configuration
- Cache is generated/temporary but shouldn't be in `build/` (often deleted)
- Triplets are project-owned customization, not third-party code

**Impact:**
- vcpkg submodule: `external/vcpkg/`
- Binary cache: `vcpkg/cache/`
- Custom triplets: `vcpkg/triplets/`
- All path references in presets and documentation updated accordingly

---

#### Decision 2: ARM64 as Primary Supported Architecture

**Options considered:**
- Default to x64 (most common developer platform)
- Default to ARM64 (current development/testing platform)
- No default (require explicit architecture selection)

**Decision:** Default presets inherit from ARM64; provide x64 presets but mark as untested.

**Rationale:**
- Current development and testing is ARM64-only (no x64 hardware available)
- Cannot claim x64 support without validation
- Honest documentation prevents user frustration
- x64 presets provided for future validation and contributor testing
- Default preset should map to tested, working configuration

**Impact:**
- Default `debug` and `release` presets inherit from `debug-arm64` and `release-arm64`
- Documentation clearly states ARM64 is tested, x64 is untested
- README includes note about architecture support status
- x64 validation deferred until x64 hardware is available

---

#### Decision 3: Remove `utility/` Scripts

**Options considered:**
- Keep scripts for backwards compatibility
- Mark scripts as deprecated
- Remove scripts entirely

**Decision:** Remove `utility/vcpkg-setup.sh` and `utility/vcpkg-teardown.sh` entirely.

**Rationale:**
- Submodule workflow replaces manual cloning
- Scripts no longer serve a purpose
- Keeping dead code increases maintenance burden
- Clear migration path: submodule initialization is standard git workflow

**Impact:**
- Documentation must guide users to submodule commands
- New bootstrap script (`script/vcpkg-bootstrap.sh`) provides convenience wrapper

---

#### Decision 4: Pin vcpkg to Specific Commit

**Options considered:**
- Track vcpkg `master` branch (floating reference)
- Pin to specific commit (fixed reference)
- Pin to specific release tag

**Decision:** Pin to specific commit (current HEAD of master at submodule addition time).

**Rationale:**
- Reproducible builds require fixed dependency versions
- vcpkg doesn't use semantic versioning or stable releases
- Specific commit ensures toolchain and ports are in sync
- Updates are explicit and testable

**Impact:**
- Submodule will be "detached HEAD" by default
- Updates require explicit `git checkout` and `git pull` in submodule
- CI and developers get identical vcpkg version

---

### 6.2 Assumptions

#### Assumption 1: Git is Available

**Assumption:** All users have git installed and are familiar with basic git workflows.

**Justification:** The project is hosted on GitHub; git is a prerequisite.

**Risk:** Users unfamiliar with submodules may struggle with initialization.

**Mitigation:** Comprehensive documentation, bootstrap script, DevContainer auto-initialization.

---

#### Assumption 2: ARM64 Development Environment Only

**Assumption:** Development and testing occur exclusively on ARM64 hardware; x64 validation is deferred.

**Justification:** Current development platform is ARM64; no x64 hardware available for testing.

**Risk:** x64 presets may have issues that won't be discovered until someone tests on x64.

**Mitigation:**
- Clearly document x64 as untested in README and preset names
- Provide x64 presets based on standard vcpkg triplets (high likelihood of working)
- Encourage community contributions for x64 validation
- Plan to validate x64 support when hardware becomes available

---

#### Assumption 3: Submodule Initialization is One-Time

**Assumption:** Users initialize submodules once and rarely interact with submodule commands.

**Justification:** Submodules are stable once initialized; updates are infrequent.

**Risk:** Submodule state issues (missing initialization, stale commits) can confuse users.

**Mitigation:**
- DevContainer auto-initializes submodules
- Bootstrap script checks and initializes if needed
- Documentation includes troubleshooting section

---

#### Assumption 4: Binary Cache is Effective

**Assumption:** vcpkg binary cache significantly reduces rebuild time after cache is populated.

**Justification:** vcpkg binary caching is designed for this purpose.

**Risk:** Large dependencies may still take time to build initially.

**Mitigation:**
- CI caches `vcpkg/cache/` across runs
- Documentation sets expectations (first build slow, subsequent builds fast)

---

#### Assumption 5: Phase 2 Will Address CI

**Assumption:** CI/CD workflow is deferred to Phase 2; manual testing suffices for Phase 1.

**Justification:** Phase 1 focuses on "golden path" template; CI is a quality-of-life enhancement.

**Risk:** CI may reveal integration issues not caught in manual testing.

**Mitigation:** Thorough manual testing with fresh clones and DevContainer.

---

## 7. Tasks

### 7.1 Git Submodule Setup

| Task ID | Description | Estimated Effort | Prerequisites |
|---------|-------------|------------------|---------------|
| VCPKG-001 | Create feature branch `feature/vcpkg-submodule` | 0.25h | None |
| VCPKG-002 | Move existing `vcpkg/` to `external/vcpkg/` (preserve .git) | 0.5h | VCPKG-001 |
| VCPKG-003 | Add vcpkg as submodule: `git submodule add https://github.com/microsoft/vcpkg.git external/vcpkg` | 0.25h | VCPKG-002 |
| VCPKG-004 | Verify submodule tracking: `git submodule status` | 0.25h | VCPKG-003 |
| VCPKG-005 | Commit submodule addition with descriptive message | 0.25h | VCPKG-004 |

**Total effort: 1.5 hours**

---

### 7.2 Directory Structure Updates

| Task ID | Description | Estimated Effort | Prerequisites |
|---------|-------------|------------------|---------------|
| VCPKG-006 | Create `external/` directory | 0.1h | VCPKG-001 |
| VCPKG-007 | Move `vcpkg-triplets/` → `external/vcpkg-triplets/` | 0.25h | VCPKG-006 |
| VCPKG-008 | Update `.gitignore`: remove `/vcpkg`, update cache path | 0.25h | VCPKG-007 |
| VCPKG-009 | Create `external/vcpkg-cache/.gitkeep` (preserve empty dir) | 0.1h | VCPKG-008 |
| VCPKG-010 | Commit directory structure changes | 0.25h | VCPKG-009 |

**Total effort: 1 hour**

---

### 7.3 CMake Presets Refactoring

| Task ID | Description | Estimated Effort | Prerequisites |
|---------|-------------|------------------|---------------|
| VCPKG-011 | Update `base-configure` toolchain path to `external/vcpkg/...` | 0.5h | VCPKG-010 |
| VCPKG-012 | Update `base-configure` overlay triplets path | 0.25h | VCPKG-011 |
| VCPKG-013 | Update `base-configure` binary cache path | 0.25h | VCPKG-012 |
| VCPKG-014 | Create `debug-x64` configure preset | 0.5h | VCPKG-013 |
| VCPKG-015 | Create `debug-arm64` configure preset | 0.5h | VCPKG-014 |
| VCPKG-016 | Create `release-x64` configure preset | 0.5h | VCPKG-015 |
| VCPKG-017 | Create `release-arm64` configure preset | 0.5h | VCPKG-016 |
| VCPKG-018 | Update `debug` preset to inherit from `debug-x64` | 0.25h | VCPKG-017 |
| VCPKG-019 | Update `release` preset to inherit from `release-x64` | 0.25h | VCPKG-018 |
| VCPKG-020 | Create corresponding build presets for x64/arm64 | 1h | VCPKG-019 |
| VCPKG-021 | Update test presets (no architecture dependency) | 0.5h | VCPKG-020 |
| VCPKG-022 | Update package presets (no architecture dependency) | 0.5h | VCPKG-021 |
| VCPKG-023 | Update workflow presets to reference new build presets | 0.5h | VCPKG-022 |
| VCPKG-024 | Validate `CMakePresets.json` schema with `cmake --list-presets` | 0.25h | VCPKG-023 |
| VCPKG-025 | Commit CMake presets changes | 0.25h | VCPKG-024 |

**Total effort: 6 hours**

---

### 7.4 DevContainer Updates

| Task ID | Description | Estimated Effort | Prerequisites |
|---------|-------------|------------------|---------------|
| VCPKG-026 | Update `postCreateCommand` to initialize submodules | 0.5h | VCPKG-025 |
| VCPKG-027 | Update `postCreateCommand` vcpkg path to `external/vcpkg/...` | 0.25h | VCPKG-026 |
| VCPKG-028 | Update `containerEnv.VCPKG_ROOT` path | 0.25h | VCPKG-027 |
| VCPKG-029 | Update `containerEnv.VCPKG_BINARY_SOURCES` path | 0.25h | VCPKG-028 |
| VCPKG-030 | Commit DevContainer changes | 0.25h | VCPKG-029 |

**Total effort: 1.5 hours**

---

### 7.5 Script Management

| Task ID | Description | Estimated Effort | Prerequisites |
|---------|-------------|------------------|---------------|
| VCPKG-031 | Create `script/vcpkg-bootstrap.sh` with submodule init logic | 1h | VCPKG-030 |
| VCPKG-032 | Make `script/vcpkg-bootstrap.sh` executable | 0.1h | VCPKG-031 |
| VCPKG-033 | Test bootstrap script on clean checkout | 0.5h | VCPKG-032 |
| VCPKG-034 | Remove `utility/vcpkg-setup.sh` | 0.1h | VCPKG-033 |
| VCPKG-035 | Remove `utility/vcpkg-teardown.sh` | 0.1h | VCPKG-034 |
| VCPKG-036 | Commit script changes | 0.25h | VCPKG-035 |

**Total effort: 2 hours**

---

### 7.6 Documentation Updates

| Task ID | Description | Estimated Effort | Prerequisites |
|---------|-------------|------------------|---------------|
| VCPKG-037 | Update `README.md` directory structure section | 0.5h | VCPKG-036 |
| VCPKG-038 | Update `README.md` getting started (local development) | 0.75h | VCPKG-037 |
| VCPKG-039 | Add `README.md` section "Working with the vcpkg Submodule" | 1h | VCPKG-038 |
| VCPKG-040 | Update `.github/copilot-instructions.md` dependency management | 0.5h | VCPKG-039 |
| VCPKG-041 | Update `.github/copilot-instructions.md` architecture selection | 0.5h | VCPKG-040 |
| VCPKG-042 | Update `.github/copilot-instructions.md` common pitfalls | 0.25h | VCPKG-041 |
| VCPKG-043 | Add `doc/ARCHITECTURE.md` dependency management section | 1.5h | VCPKG-042 |
| VCPKG-044 | Review all documentation for old `vcpkg/` path references | 0.5h | VCPKG-043 |
| VCPKG-045 | Commit documentation changes | 0.25h | VCPKG-044 |

**Total effort: 5.75 hours**

---

### 7.7 Testing and Validation

| Task ID | Description | Estimated Effort | Prerequisites |
|---------|-------------|------------------|---------------|
| VCPKG-046 | Test 1: Fresh clone workflow (arm64) | 1h | VCPKG-045 |
| VCPKG-047 | Test 2: DevContainer workflow | 1h | VCPKG-046 |
| VCPKG-048 | Test 3: Architecture selection (arm64 presets) | 0.75h | VCPKG-047 |
| VCPKG-049 | Test 4: Binary cache effectiveness | 0.75h | VCPKG-048 |
| VCPKG-050 | Test 5: Submodule update workflow | 0.5h | VCPKG-049 |
| VCPKG-051 | Test 6: Obsolete script removal verification | 0.5h | VCPKG-050 |
| VCPKG-052 | Regression test: Run full test suite | 0.25h | VCPKG-051 |
| VCPKG-053 | Fix any issues discovered during testing | 2h (buffer) | VCPKG-052 |

**Total effort: 6.75 hours**

---

### 7.8 Finalization

| Task ID | Description | Estimated Effort | Prerequisites |
|---------|-------------|------------------|---------------|
| VCPKG-055 | Review all changes for consistency and completeness | 1h | VCPKG-054 |
| VCPKG-056 | Update `doc/implementation_plan_phase_1.md` status (mark Phase 1 complete) | 0.25h | VCPKG-055 |
| VCPKG-057 | Create pull request with comprehensive description | 0.5h | VCPKG-056 |
| VCPKG-058 | Address PR review feedback (buffer) | 2h (buffer) | VCPKG-057 |
| VCPKG-059 | Merge to main branch | 0.25h | VCPKG-058 |

**Total effort: 4 hours**

---

### 7.9 Task Summary

| Phase | Task Count | Total Effort |
|-------|------------|--------------|
| Git Submodule Setup | 5 | 1.5h |
| Directory Structure Updates | 5 | 1h |
| CMake Presets Refactoring | 15 | 6h |
| DevContainer Updates | 5 | 1.5h |
| Script Management | 6 | 2h |
| Documentation Updates | 9 | 5.75h |
| Testing and Validation | 8 | 6.75h |
| Finalization | 5 | 4h |
| **TOTAL** | **58** | **28.5 hours** |

**Note:** Effort estimates include time for implementation, testing, and iteration. Actual time may vary based on issues encountered.

---

## 8. Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Submodule initialization failures in CI | Medium | High | Test CI workflow thoroughly; provide clear error messages |
| Users forget to initialize submodules | High | High | DevContainer auto-initializes; documentation includes troubleshooting |
| Architecture confusion (wrong preset selected) | Medium | Medium | Clear preset naming; documentation explains selection |
| Binary cache doesn't persist in CI | Low | Medium | Use GitHub Actions cache action with correct paths |
| Existing builds break after path changes | Low | High | Thorough testing with clean clones; regression tests |
| Documentation becomes outdated | Medium | Low | Consolidate submodule instructions in one README section |

---

## 9. Success Criteria

This implementation is considered successful when:

1. ✅ A fresh clone of the repository can be configured, built, and tested without manual vcpkg setup (on ARM64)
2. ✅ DevContainer opens and bootstraps without errors
3. ✅ ARM64 preset works correctly; x64 preset is provided but documented as untested
4. ✅ Binary cache reduces rebuild time significantly (>50% faster with warm cache)
5. ✅ All existing tests pass without modification
6. ✅ Documentation accurately describes submodule workflow and architecture support status
7. ✅ Obsolete scripts are removed and references updated
8. ✅ Submodule update workflow is documented and tested
9. ✅ `.gitmodules` is tracked and submodule is pinned to specific commit
10. ✅ Phase 1 acceptance criteria (PRD §9) are met

---

## 10. Post-Implementation Notes

**To be completed after implementation:**

- Actual effort spent per task
- Issues encountered and resolutions
- Lessons learned
- Recommendations for Phase 2

---

## 11. Appendix: Command Reference

### Fresh Clone Workflow

```bash
# Clone with submodules
git clone --recurse-submodules <repo-url>

# Or clone then initialize
git clone <repo-url>
cd cpp-app-template
git submodule update --init --recursive

# Bootstrap vcpkg
./script/vcpkg-bootstrap.sh

# Configure, build, test
cmake --preset debug
cmake --build --preset debug-build
ctest --preset debug-test --output-on-failure
```

### Submodule Management

```bash
# Check submodule status
git submodule status

# Update submodule to latest master
cd external/vcpkg
git checkout master
git pull origin master
cd ../..
git add external/vcpkg
git commit -m "chore: update vcpkg"

# Reset submodule to tracked commit
git submodule update --init --recursive
```

### Architecture Selection

```bash
# ARM64 (default, tested)
cmake --preset debug
cmake --build --preset debug-build

# ARM64 (explicit)
cmake --preset debug-arm64
cmake --build --preset debug-arm64-build

# x64 (untested, requires x64 hardware)
cmake --preset debug-x64
cmake --build --preset debug-x64-build
```

### Troubleshooting

```bash
# Submodule not initialized
git submodule update --init --recursive

# vcpkg executable missing
./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics

# Clean rebuild
rm -rf build vcpkg/cache
cmake --preset debug
cmake --build --preset debug-build

# Check triplet selection
grep VCPKG_TARGET_TRIPLET build/debug/CMakeCache.txt
```

---

**End of Implementation Plan**
