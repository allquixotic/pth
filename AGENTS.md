# GNU Pth Modernization Plan

## 1. Guiding Principles

- **API Compatibility**: Maintain 100% API and ABI compatibility with the existing GNU Pth 2.0.7 release. The behavior of all public functions should remain unchanged.
- **Modern Linux Focus**: The primary target for modernization is modern Linux distributions using recent versions of glibc. The goal is to ensure stable and predictable behavior on these systems.
- **Retain Portability**: While focusing on modern Linux, the changes should be implemented in a way that retains portability to other modern Unix-like systems (e.g., FreeBSD, macOS).
- **Incremental Changes**: The modernization process will be broken down into distinct phases to ensure that the library remains in a buildable and testable state at each stage.

## 2. Plan of Action

### Phase 1: Build System Overhaul

The current build system is based on an outdated version of Autoconf and includes a number of platform-specific workarounds for systems that are no longer in common use. This phase will focus on modernizing the build system to provide a stable foundation for the rest of the modernization effort.

- **Upgrade Autotools**: Convert the `configure.ac` and `Makefile.in` files to use a modern version of Autoconf (2.69 or newer) and Libtool. This will involve replacing obsolete macros and updating the build process to follow current best practices.
- **Remove Obsolete Platform Support**: Remove any build-time checks and workarounds for obsolete or esoteric platforms that are no longer relevant. The focus will be on creating a clean, maintainable build configuration for modern systems.
- **Update `shtool`**: The bundled `shtool` script will be replaced with a modern equivalent or its usage will be replaced with standard shell commands.
- **Integrate a Test Harness**: The `Makefile.am` will be updated to include a proper `make check` target that builds and runs the entire test suite. This will provide a simple, standardized way to verify the library's functionality.

### Phase 2: Core Implementation and Compatibility Fixes

This phase will address the core compatibility issues that are causing the library to fail on modern systems. The focus will be on the machine context switching mechanism and the API wrappers for system calls.

- **Stabilize Machine Context Switching**: The machine context switching implementation in `pth_mctx.c` will be thoroughly reviewed and updated to work reliably on modern glibc. This will likely involve:
    - Prioritizing the use of `sigsetjmp`/`siglongjmp` over the deprecated `makecontext`/`swapcontext`.
    - Verifying the correct handling of signal masks and stack pointers during context switches.
    - Creating a series of minimal, targeted test cases to isolate and validate the context switching mechanism.
- **Review and Update API Wrappers**: The POSIX API wrappers in `pth_high.c` and `pth_syscall.c` will be reviewed to ensure that they correctly handle the `errno` values and behavior of modern Linux system calls.
- **Adopt Modern C Standards**: The codebase will be updated to adhere to the C99 standard. This will involve updating function declarations, replacing non-standard library functions with their standard equivalents, and using types from `<stdint.h>` where appropriate.
- **Address User-Provided Patches**: The existing `git diff` will be reviewed and the changes will be integrated into the codebase.

### Phase 3: Test Suite Enhancement

The existing test suite provides a good starting point, but it needs to be expanded to provide more comprehensive coverage and to test for the specific issues that have been reported on modern systems.

- **Develop New Test Cases**: New test cases will be developed to target the following areas:
    - **Context Switching**: Stress tests to verify the stability of the context switching mechanism under heavy load.
    - **Signal Handling**: Tests to ensure that signals are handled correctly and that the per-thread signal masks are working as expected.
    - **I/O and Timers**: Tests to verify the correct behavior of the I/O and timer event sources, especially in combination with other event types.
- **Create a Continuous Integration (CI) Configuration**: A CI configuration file (e.g., for GitHub Actions) will be created to automatically build and test the library on a variety of modern Linux distributions.

### Phase 4: Documentation

The final phase will involve updating the library's documentation to reflect the changes made during the modernization process.

- **Update `README`, `INSTALL`, and `PORTING`**: These files will be updated to reflect the new build process and the list of supported platforms.
- **Update Manual Pages**: The POD manual pages will be reviewed and updated to ensure that they accurately describe the library's API and behavior.

## 3. Immediate Next Steps

1.  **Analyze `git diff`**: Review the user's uncommitted changes to understand the fixes that have already been implemented and to identify any new test cases.
2.  **Begin Build System Overhaul**: Start Phase 1 by creating a new `configure.ac` and `Makefile.am` that use a modern version of Autotools.

