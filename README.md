# Sail

[![ci](https://github.com/cplusplus-lang/Sail/actions/workflows/ci.yml/badge.svg)](https://github.com/cplusplus-lang/Sail/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/cplusplus-lang/Sail/branch/main/graph/badge.svg)](https://codecov.io/gh/cplusplus-lang/Sail)
[![CodeQL](https://github.com/cplusplus-lang/Sail/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/cplusplus-lang/Sail/actions/workflows/codeql-analysis.yml)

## About Sail

Sail is a modern C++ package manager and build tool inspired by Rust's Cargo. It provides a simple, intuitive interface for creating, building, and managing C++ projects with automatic dependency management using CMake and CPM (CMake Package Manager) under the hood.

## Features

- 🚀 **Easy Project Creation**: Create new C++ projects with a simple command
- 📦 **Dependency Management**: Add and manage dependencies with automatic version resolution
- 🔨 **Cross-Platform Building**: Uses CMake behind the scenes for reliable cross-platform builds
- 🎯 **Cargo-like Interface**: Familiar commands for developers coming from Rust
- 📋 **TOML Configuration**: Simple `Sail.toml` configuration files
- 🧹 **Clean Builds**: Organized build artifacts in `target/debug` and `target/release` directories

## Installation

Build Sail from source:

```bash
git clone <repository-url>
cd sail-dev

# Debug build
mkdir -p target/debug
cd target/debug
cmake ../.. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Or Release build  
mkdir -p target/release
cd target/release
cmake ../.. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

The `sail` executable will be available at:
- `target/debug/sail` (debug build)
- `target/release/sail` (release build)

## Quick Start

### Create a New Project

```bash
sail new my-project
cd my-project
```

This creates a new C++ project with the following structure:
```
my-project/
├── Sail.toml
└── src/
    └── main.cpp
```

### Initialize in Existing Directory

```bash
sail init
```

### Add Dependencies

Add popular C++ libraries with automatic version resolution:

```bash
# Add with default version
sail add fmt
sail add spdlog
sail add catch2
sail add cli11
sail add nlohmann_json

# Add with specific version
sail add fmt@10.1.1
sail add spdlog@1.13.0
```

### Build and Run

```bash
# Build the project
sail build

# Build and run in one command
sail run
```

### Clean Build Artifacts

```bash
sail clean
```

### Run Tests

```bash
sail test
```

## Commands

| Command | Description |
|---------|-------------|
| `sail new <name>` | Create a new Sail package |
| `sail init` | Initialize a Sail package in the current directory |
| `sail add <dependency>` | Add a dependency to the current package |
| `sail build` | Compile the current package |
| `sail run` | Build and run the current package |
| `sail clean` | Remove the target directory |
| `sail test` | Run tests |
| `sail --version` | Show version information |
| `sail --help` | Show help information |

## Configuration

### Sail.toml

The `Sail.toml` file contains project metadata and dependencies:

```toml
[package]
name = "my-project"
version = "0.1.0"
authors = ["Your Name <your.email@example.com>"]

[dependencies]
fmt = "10.1.1"
spdlog = "1.13.0"
catch2 = "3.4.0"
```

### Supported Dependencies

Sail comes with built-in support for popular C++ libraries:

- **fmt**: Modern C++ formatting library
- **spdlog**: Fast C++ logging library  
- **catch2**: Modern C++ testing framework
- **cli11**: Command line parser for C++
- **nlohmann_json**: JSON for Modern C++

## Project Structure

Sail organizes projects with a clean directory structure:

```
my-project/
├── Sail.toml           # Project configuration
├── src/                # Source files
│   └── main.cpp
└── target/             # Build artifacts (auto-generated)
    └── cmake/          # CMake build files
        ├── CMakeLists.txt
        ├── cmake/
        │   └── CPM.cmake
        └── build/      # Compiled binaries
```

## Sail Development Structure

When working on Sail itself, the project structure is:

```
sail-dev/
├── README.md           # Project documentation
├── src/
│   └── main.cpp        # Sail implementation
├── test/
│   ├── CMakeLists.txt  # Test configuration
│   ├── test_sail.cpp   # Comprehensive Sail tests
│   ├── tests.cpp       # Unit tests
│   └── constexpr_tests.cpp
└── target/
    ├── debug/          # Debug build artifacts
    │   └── sail        # Debug sail executable
    └── release/        # Release build artifacts
        └── sail        # Release sail executable
```

## How It Works

1. **Project Configuration**: `Sail.toml` defines your project metadata and dependencies
2. **CMake Generation**: Sail automatically generates optimized `CMakeLists.txt` files
3. **Dependency Resolution**: Uses CPM to fetch and build dependencies from GitHub
4. **Cross-Platform Building**: Leverages CMake for reliable builds across platforms
5. **Clean Organization**: All build artifacts are contained in the `target/cmake` directory for user projects, and `target/debug`/`target/release` for Sail development

## Examples

### Creating a Console Application

```bash
sail new hello-world
cd hello-world
sail add fmt
```

Edit `src/main.cpp`:
```cpp
#include <fmt/core.h>

int main() {
    fmt::print("Hello, World!\n");
    return 0;
}
```

```bash
sail run
```

### Adding Multiple Dependencies

```bash
sail add fmt@10.1.1
sail add spdlog@1.13.0  
sail add catch2@3.4.0
```

Your `Sail.toml` will be automatically updated:
```toml
[dependencies]
fmt = "10.1.1"
spdlog = "1.13.0"
catch2 = "3.4.0"
```

## Comparison with Cargo

| Feature | Cargo (Rust) | Sail (C++) |
|---------|--------------|------------|
| New Project | `cargo new` | `sail new` |
| Add Dependency | `cargo add` | `sail add` |
| Build | `cargo build` | `sail build` |
| Run | `cargo run` | `sail run` |
| Test | `cargo test` | `sail test` |
| Clean | `cargo clean` | `sail clean` |
| Config File | `Cargo.toml` | `Sail.toml` |
| Build System | Built-in | CMake + CPM |

## Development and Testing

### Running Tests

Sail includes a comprehensive test suite that validates all commands and functionality:

```bash
# Build the project with tests (Debug build recommended for testing)
mkdir -p target/debug
cd target/debug
cmake ../.. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Run all tests
ctest

# Run only the comprehensive Sail functionality tests  
ctest -R sail.comprehensive_test

# Run tests with verbose output
ctest --output-on-failure

# Alternative: Run tests from project root
# cmake --build target/debug --target test
```

### Test Coverage

The test suite includes:

- **Unit tests** (`test/tests.cpp`) - Basic functionality tests using Catch2
- **CLI tests** (`test/CMakeLists.txt`) - Command-line interface validation
- **Comprehensive tests** (`test/test_sail.cpp`) - Full workflow testing (new → add → build → run → clean)
- **Error handling** - Invalid input and edge case testing
- **Cross-platform compatibility** - Tests work on Windows, macOS, and Linux

All tests are automatically run in CI/CD and must pass before merging changes.

### Contributing

1. Make changes to the source code
2. Run the test suite to ensure nothing is broken
3. Add new tests for new functionality
4. Update documentation as needed

## More Details

 * [Dependency Setup](README_dependencies.md)
 * [Building Details](README_building.md)
 * [Troubleshooting](README_troubleshooting.md)
 * [Docker](README_docker.md)
