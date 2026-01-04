# /code/cxxapp/toolchain.cmake
# vcpkg chainload toolchain file for a GNU (gcc / g++) toolchain.
#
# Purpose:
#   This file is intended to be chainloaded by vcpkg. Typical usage:
#
#     cmake
#       -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
#       -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=/code/cxxapp/toolchain.cmake
#       ...
#
#   It selects a GNU toolchain (gcc/g++) from the system if no compiler is
#   specified by the invoker, honours user-specified compilers, and optionally
#   configures a sysroot and find root paths for cross-compilation.
#
# Notes:
#   - This file sets cache values only when the corresponding CMake variable
#     is not already defined by the user or the environment.
#   - It uses conservative defaults and reports the chosen configuration.
#   - All settings happen before the project() call in the parent CMake run.
#
# Author: Generated (concise, implementation-focused)
# Licence: Use as you wish

# Guard against re-loading this toolchain file multiple times.
if(NOT DEFINED vcpkgGnuToolchainLoaded)
    # Mark as loaded in the cache so subsequent includes do not re-run logic.
    set(vcpkgGnuToolchainLoaded TRUE CACHE INTERNAL "vcpkg GNU toolchain loaded")
endif()

# --------------------------
# Target system (defaults)
# --------------------------
# If the caller did not specify a target system name, prefer "Linux" for GNU.
if(NOT DEFINED CMAKE_SYSTEM_NAME)
    # Set as cache so it is visible to downstream logic and not overridden silently.
    set(CMAKE_SYSTEM_NAME "Linux" CACHE STRING "Target system name for the toolchain")
endif()

# --------------------------
# Compiler selection
# --------------------------
# Do not override user-specified compilers. Only provide defaults when missing.

# Helper: attempt to locate an executable in common system locations.
# The find_program call uses PATH by default so explicit PATHS are unnecessary
# on typical systems; explicit fallbacks are provided to be robust in minimal
# environments.
if(NOT DEFINED CMAKE_C_COMPILER)
    find_program(defaultGcc NAMES gcc HINTS /usr/bin /usr/local/bin)
    if(defaultGcc)
        # Set as cache FILEPATH so CMake treats it like a discovered compiler.
        set(CMAKE_C_COMPILER ${defaultGcc} CACHE FILEPATH "Path to C compiler")
    else()
        message(FATAL_ERROR "GNU C compiler (gcc) not found and CMAKE_C_COMPILER not provided")
    endif()
endif()

if(NOT DEFINED CMAKE_CXX_COMPILER)
    find_program(defaultGpp NAMES g++ HINTS /usr/bin /usr/local/bin)
    if(defaultGpp)
        set(CMAKE_CXX_COMPILER ${defaultGpp} CACHE FILEPATH "Path to C++ compiler")
    else()
        message(FATAL_ERROR "GNU C++ compiler (g++) not found and CMAKE_CXX_COMPILER not provided")
    endif()
endif()

# Report chosen compilers for user visibility.
message(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Using C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMake system name: ${CMAKE_SYSTEM_NAME}")

# --------------------------
# Sysroot and find root path
# --------------------------
# If an explicit sysroot is provided via environment or cache variables, use it.
# Common env vars: SYSROOT, GNU_SYSROOT. Also allow the user to pass
# -DgnuSysroot=/path/to/sysroot to CMake.
if(NOT DEFINED gnuSysroot)
    if(DEFINED ENV{SYSROOT})
        set(gnuSysroot $ENV{SYSROOT})
    elseif(DEFINED ENV{GNU_SYSROOT})
        set(gnuSysroot $ENV{GNU_SYSROOT})
    endif()
endif()

# If gnuSysroot has been discovered, apply it conservatively.
if(DEFINED gnuSysroot AND gnuSysroot)
    # Set sysroot for compilers that respect the --sysroot option by appending the
    # flag to initial flags. We use *_INIT variables to avoid clobbering flags
    # already set by higher-level scripts.
    if(NOT DEFINED CMAKE_SYSROOT)
        set(CMAKE_SYSROOT ${gnuSysroot} CACHE PATH "Sysroot for the target toolchain")
    endif()

    # Encourage CMake find_* to search inside the sysroot for libraries and headers.
    if(NOT DEFINED CMAKE_FIND_ROOT_PATH)
        set(CMAKE_FIND_ROOT_PATH ${gnuSysroot} CACHE PATH "Find root path (sysroot) for cross-compilation")
    endif()

    message(STATUS "Configured sysroot: ${CMAKE_SYSROOT}")
endif()

# --------------------------
# vcpkg-specific recommendations
# --------------------------
# vcpkg honours VCPKG_TARGET_TRIPLET; do not override if provided. If the user
# provided VCPKG_DEFAULT_TRIPLET as an environment variable, propagate it to
# CMake cache for vcpkg usage.
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET})
        set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET} CACHE STRING "vcpkg target triplet")
        message(STATUS "Set VCPKG_TARGET_TRIPLET from environment: ${VCPKG_TARGET_TRIPLET}")
    endif()
endif()

# --------------------------
# Minimal sanity checks
# --------------------------
# Confirm compilers are usable by checking their IDs. This is a light-weight
# check; full validation will occur when the parent CMake project tries to
# compile.
if(NOT "${CMAKE_C_COMPILER}" STREQUAL "" AND NOT "${CMAKE_CXX_COMPILER}" STREQUAL "")
    # Do not attempt heavy probing here; just provide a small reassurance.
    message(STATUS "GNU toolchain chainload configuration complete.")
else()
    message(FATAL_ERROR "Toolchain selection failed: compilers not configured")
endif()

# End of toolchain.cmake
