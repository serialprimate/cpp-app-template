# cpp-app-template

A modern, production-ready C++23 application template with modular architecture, testing infrastructure, and DevContainer support.

---

## Features

- **Modern C++23** with strict warnings and error checking
- **Modular structure**: Separate directories for apps, libraries, and tests
- **CMake presets** for reproducible builds (debug, release, CI)
- **vcpkg manifest mode** for hermetic dependency management
- **GoogleTest** integration for unit and integration testing
- **DevContainer** with automatic vcpkg bootstrap
- **Quality tooling**: clang-format, clang-tidy
- **Interface + Factory pattern** for clean encapsulation

---

## Directory Structure

```
cpp-app-template/
├── external/               # Third-party dependencies (submodules only)
│   └── vcpkg/              # vcpkg submodule (dependency manager)
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
│       └── src/            # Private implementation
├── test/                   # Tests
│   ├── unit/               # Unit tests (library-level)
│   │   └── loggerUnitTest/
│   └── int/                # Integration tests (scenario-based)
│       └── appIntTest/
├── doc/                    # Documentation
│   └── ARCHITECTURE.md     # Architecture guide
├── script/                 # Helper scripts
│   ├── clang-format-all.sh
│   └── clang-tidy-all.sh
├── vcpkg/                  # Project vcpkg configuration
│   ├── cache/              # Binary cache (not committed)
│   └── triplets/           # Overlay triplets for custom toolchain
└── CMakePresets.json       # Build configuration presets
```

---

## Getting Started

### Prerequisites

- **Linux** (Ubuntu 24.04 LTS or similar)
- **VS Code** with DevContainer support (recommended)
- **CMake** 3.28.3 or later
- **C++23-capable compiler** (GCC 13+ or Clang 17+)

### Option 1: DevContainer (Recommended)

1. Open the repository in VS Code
2. Accept the prompt to reopen in DevContainer
3. vcpkg will bootstrap automatically

### Option 2: Local Development

1. **Bootstrap vcpkg:**
   ```bash
   git submodule update --init --recursive
   ./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics
   ```

   Or use the convenience script:
   ```bash
   ./script/vcpkg-bootstrap.sh
   ```

2. **Configure the project:**
   ```bash
   cmake --preset debug
   ```

3. **Build:**
   ```bash
   cmake --build --preset debug-build
   ```

4. **Run tests:**
   ```bash
   ctest --preset debug-test --output-on-failure
   ```

5. **Run the sample application:**
   ```bash
   ./build/debug/app/sampleApp/sampleApp
   ```

---

## Build Presets

### Configure Presets

| Preset | Description |
|:-------|:------------|
| `lint` | Linting build with clang-tidy |
| `debug` | Debug build with symbols |
| `release` | Optimised release build with LTO |
| `ci` | CI/CD preset (release + tests enabled) |

### Build Presets

| Preset | Description |
|:-------|:------------|
| `lint-build` | Build linting configuration |
| `lint-rebuild` | Clean rebuild (lint) |
| `debug-build` | Build debug configuration |
| `debug-rebuild` | Clean rebuild (debug) |
| `release-build` | Build release configuration |
| `ci-build` | Build CI configuration |

### Test Presets

| Preset | Description |
|:-------|:------------|
| `debug-test` | Run tests in debug mode |
| `release-test` | Run tests in release mode |
| `ci-test` | Run CI tests |

### Workflow Presets

Workflows chain configure → build → test → package:

```bash
cmake --workflow --preset debug-workflow
cmake --workflow --preset release-workflow
cmake --workflow --preset ci-workflow
```

---

## Testing

This template includes both unit and integration tests using GoogleTest.

### Unit Tests

Located in `test/unit/`, unit tests verify individual library components in isolation.

Example:
```bash
# Run all tests
ctest --preset debug-test --output-on-failure

# Run specific test
./build/debug/test/unit/loggerUnitTest/loggerUnitTest --gtest_filter=LoggerFactoryTest.*
```

### Integration Tests

Located in `test/int/`, integration tests verify interactions between multiple components.

### Writing Tests

See the existing tests in `test/unit/loggerUnitTest/src/LoggerFactoryTest.cpp` and `test/int/appIntTest/src/AppIntTest.cpp` for examples.

---

## Code Quality

### Formatting

Format all C++ files:
```bash
./script/clang-format-all.sh
```

### Linting

Lint all C++ files:
```bash
./script/clang-tidy-all.sh
```

Or build with clang-tidy enabled:
```bash
cmake --preset lint
cmake --build --preset lint-build
```

---

## Adding New Components

### Adding a Library

1. Create directory structure:
   ```bash
   mkdir -p lib/mylib/include/mylib
   mkdir -p lib/mylib/src
   ```

2. Define public interface in `lib/mylib/include/mylib/IMyLib.hpp`
3. Define factory in `lib/mylib/include/mylib/MyLibFactory.hpp`
4. Implement privately in `lib/mylib/src/`
5. Create `lib/mylib/CMakeLists.txt` (follow `lib/logger/CMakeLists.txt` pattern)
6. Add to root `CMakeLists.txt`: `add_subdirectory(lib/mylib)`

See [doc/ARCHITECTURE.md](doc/ARCHITECTURE.md) for detailed guidance.

### Adding Tests

1. **Unit tests**: Create `test/unit/mylibUnitTest/` following `test/unit/loggerUnitTest/` pattern
2. **Integration tests**: Create `test/int/mycomponentIntTest/` following `test/int/appIntTest/` pattern for cross-component testing
3. Wire into root `CMakeLists.txt` inside the `if(BUILD_TESTING)` block

---

## Architecture

This template follows modern C++ design principles:

- **Interface + Factory pattern** for encapsulation
- **Composition root** (in `main.cpp`) for dependency wiring
- **Dependency Injection** for testability
- **SOLID principles** throughout

See [doc/ARCHITECTURE.md](doc/ARCHITECTURE.md) for the complete architecture guide.

---

## Working with the vcpkg Submodule

This project uses vcpkg as a git submodule to ensure reproducible builds.

### Fresh Clone

Initialise the submodule after cloning:

```bash
git clone <repo-url>
cd cpp-app-template
git submodule update --init --recursive
```

Or clone with submodules in one step:

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

**Note:** x64 presets are provided for future compatibility but have not been tested on x64 hardware.

---

## Dependencies

Managed via **vcpkg manifest mode** with pinned baseline.

Current dependencies:
- **fmt**: String formatting library
- **GoogleTest**: Testing framework

To add a new dependency:

1. Edit `vcpkg.json`:
   ```json
   {
     "dependencies": [
       "fmt",
       "gtest",
       "your-package"
     ]
   }
   ```

2. Update your `CMakeLists.txt`:
   ```cmake
   find_package(YourPackage CONFIG REQUIRED)
   target_link_libraries(yourTarget PRIVATE YourPackage::YourPackage)
   ```

3. Reconfigure:
   ```bash
   cmake --preset debug
   ```

---

## Packaging

Generate distributable packages:

```bash
# Debug package
cpack --preset debug-package

# Release package
cpack --preset release-package
```

Packages are created in `build/<preset>/package/` as `.tar.gz` archives.

---

## Troubleshooting

### vcpkg bootstrap fails

```bash
rm -rf vcpkg/cache/
./external/vcpkg/bootstrap-vcpkg.sh -disableMetrics

# Or:
./script/vcpkg-bootstrap.sh
```

### Build directory is corrupted

```bash
rm -rf build/
cmake --preset debug
```

### Tests not discovered

Ensure `BUILD_TESTING` is enabled (default ON):
```bash
cmake --preset debug -DBUILD_TESTING=ON
```

### clang-tidy errors

Update to latest baseline or disable specific checks in `.clang-tidy`:
```yaml
Checks: '-modernize-use-trailing-return-type'
```

---

## Platform Support

**Linux only**. This template targets Linux (Ubuntu 24.04 LTS) and uses GNU toolchain (`arm64-linux-gnu` triplet). Support for other platforms is not provided.

---

## Documentation

- [Architecture Guide](doc/ARCHITECTURE.md) - Design patterns and principles

---

## References

### CMake & Build Systems
- [VS Code CMake Tools](https://code.visualstudio.com/docs/cpp/CMake-linux)
- [CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)

### Code Quality
- [Clang-Tidy](https://clang.llvm.org/extra/clang-tidy/)
- [Clang-Format](https://clang.llvm.org/docs/ClangFormat.html)

### C++ Standards
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [C++23 Features](https://en.cppreference.com/w/cpp/23)

---

## Licence

MIT (or specify your licence)

---

**Template Version**: 2.0.0
**Last Updated**: 2026-01-01
