---
name: C++ Development Instructions
description: Modern C++23 standards and best practices
applyTo: "**/*.cpp,**/*.cc,**/*.cxx,**/*.hpp,**/*.h,**/*.hxx"
---

# C++ Development Instructions

## Code Style

- Follow **LLVM/Clang** coding style as the baseline
- Use **clang-format** to enforce code style automatically

## Naming Conventions (LLVM/Clang-aligned, with PascalCase type filenames)

- **Filenames:**
  - For files that define a primary type/class/struct/enum, use PascalCase (e.g., `MappedFile.hpp`, `MappedFile.cpp`).
  - For utility, test, or non-type files, use PascalCase with descriptive suffix (e.g., `LoggerFactoryTest.cpp`, `AppIntTest.cpp`).
- **Types (class/struct/enum):** PascalCase (e.g., `MappedFile`, `ExitCode`).
- **Functions:** camelCase (e.g., `openReadonly`, `reset`, `printUsage`).
- **Variables:** camelCase (e.g., `filePath`, `mappingPtr`, `bufferSize`).
- **Constants:** PascalCase or ALL_CAPS (e.g., `MaxBufferSize`, `DEFAULT_TIMEOUT`).
- **Macros:** ALL_CAPS with underscores (e.g., `MYPROJ_TEST_CASE`).
- **Namespaces:** camelCase preferred (e.g., `myProj`, `testHelpers`).
- **Test naming:**
  - Test executables: `<lib-name>UnitTest` for unit tests, `<component-name>IntTest` for integration tests.
  - Test suites (GTest fixtures): `<class-name>Test` for unit tests (e.g., `LoggerFactoryTest`), descriptive name ending in `Suite` using `Int` abbreviation for integration tests (e.g., `AppLoggingIntSuite`).
  - Test cases: Use descriptive names that indicate what is being tested (e.g., `CreateDefaultLoggerReturnsNonNull`, `ApplicationStartupSequence`).

> These conventions match LLVM/Clang defaults, except for PascalCase filenames for type/class headers and implementations. Consistency is required across all new and refactored code.

## Language Standards & Features
- Use **C++23** as the default standard (or C++20 as minimum)
- Prefer modern C++ styles and idioms over legacy C-style approaches
- Leverage standard library features (STL) before third-party libraries
- Use concepts for template constraints when applicable
- Adopt ranges library for collection operations

## Memory Management & Ownership
- Follow **RAII** (Resource Acquisition Is Initialization) principles strictly
- Avoid raw pointers for ownership; use smart pointers:
  - `std::unique_ptr` for exclusive ownership
  - `std::shared_ptr` only when shared ownership is necessary
  - `std::weak_ptr` to break circular references
- Prefer stack allocation over heap allocation when possible
- Use `std::make_unique` and `std::make_shared` for smart pointer creation
- Avoid manual `new`/`delete` - let destructors handle cleanup

## Type Safety & Correctness
- Enable and maintain **const correctness** throughout
- Mark member functions `const` when they don't modify object state
- Use `constexpr` for compile-time computations
- Prefer `enum class` over plain `enum` for type-safe enumerations
- Use `std::optional` for potentially absent values instead of pointers or magic values
- Use `std::variant` for type-safe unions
- Use `std::expected` (C++23) for error handling with values
- Use `std::mdspan` (C++23) for multi-dimensional array views
- Consider `if consteval` (C++23) for compile-time vs runtime branching

## Strings & Text
- Use `std::string` for owned string data
- Use `std::string_view` for non-owning, read-only string parameters
- Prefer `std::format` (C++20) or `fmt` library for string formatting
- Use `std::filesystem::path` for file path manipulation

## Code Organization
- Use `#pragma once` for include guards
- Organize code in namespaces to avoid name collisions
- Keep header files minimal - include only what's necessary
- Prefer forward declarations in headers when possible
- Use `inline` or header-only implementations judiciously

## Error Handling
- Use exceptions for exceptional conditions, not normal control flow
- Create domain-specific exception hierarchies derived from `std::exception`
- Use RAII to ensure exception safety
- Consider `noexcept` specification for functions that guarantee no exceptions
- Document exception specifications in function contracts

## Modern Idioms & Best Practices
- Use `auto` for complex types and obvious initializations
- Use **trailing return types** (`auto func() -> Type`) for consistency with modern C++ style and to satisfy linting requirements
- Use structured bindings for tuple/pair decomposition
- Prefer range-based for loops over index-based iteration
- Use lambda expressions for short, local operations
- Move semantics: implement move constructors/assignment for resource-owning types
- Delete copy operations explicitly if copying doesn't make sense
- Use `[[nodiscard]]` attribute for functions where ignoring the return value is an error
- Ensure all used types (e.g., `std::size_t`, `std::string`) have their defining headers explicitly included (e.g., `<cstddef>`, `<string>`), even if transitively available

## Classes & Interfaces
- Follow the **Rule of Five** (or Rule of Zero): if a class defines a destructor, it should also define or delete copy/move constructors and assignment operators
- For interface classes (with virtual destructors), explicitly `= default` the default constructor and `= delete` copy/move operations to prevent slicing and ensure correct lifecycle management
- Member variables should be **private**. Use protected or public accessors if needed, even in test fixtures

## Concurrency
- Use `std::thread`, `std::async`, or C++20 coroutines for multithreading
- Prefer `std::mutex`, `std::lock_guard`, and `std::unique_lock` for synchronization
- Use `std::atomic` for lock-free operations on simple types
- Consider thread-safe data structures or message passing over shared mutable state

## Testing & Quality
- Write unit tests using modern frameworks (Google Test, Catch2)
- Use sanitizers (AddressSanitizer, UndefinedBehaviorSanitizer) during development
- Enable high warning levels (`-Wall -Wextra -Wpedantic`) and treat warnings as errors
- Use static analysis tools (clang-tidy, cppcheck)

## API Documentation ("Modern Markdown" style)
- Use `///` for API documentation comment blocks
- The first line is a summary statement
- Use Markdown headers (e.g., `## Parameters`, `## Returns`, `## Throws`, etc)
- Reference parameters/variables/identifiers using backticks (e.g., `param1`)
- DO NOT use Doxygen tags
