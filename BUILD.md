# Building Atlas (Non-Nix Users)

This document provides build instructions for developers who are not using Nix. **Please note**: Nix is the first-class citizen build environment for Atlas, and these instructions are provided as a convenience for developers who cannot or prefer not to use Nix.

## Prerequisites

Atlas is a modern C++23 game engine that requires cutting-edge compiler support. Before building, ensure your system meets these requirements:

### Compiler Requirements

**Minimum Required:**
- **Clang 18+** or **GCC 14+** with full C++23 support
- Support for `std::print`, `std::ranges::to`, and other C++23 features

**Recommended:**
- **Clang 20** (same as used in the Nix environment)
- **LLVM/Clang tools** for development

### Platform-Specific Setup

#### Linux (Ubuntu 24.04+, Debian 12+, or equivalent)

```bash
# Install modern compiler (if not available, consider using a PPA or newer distro)
sudo apt update
sudo apt install clang-18 clang++-18 cmake ninja-build git

# Set as default (optional)
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100
```

#### macOS

```bash
# Install Xcode Command Line Tools or full Xcode
xcode-select --install

# Or install via Homebrew for latest LLVM
brew install llvm cmake ninja
```

#### Windows

**Option 1: Visual Studio 2022 (17.8+)**
- Install Visual Studio 2022 with C++ workload
- Ensure C++23 standard library support is enabled

**Option 2: Clang/LLVM**
```powershell
# Using winget
winget install LLVM.LLVM
winget install Kitware.CMake
winget install Ninja-build.Ninja

# Or using Chocolatey
choco install llvm cmake ninja
```

## Building Atlas

### Step 1: Clone and Setup Dependencies

```bash
# Clone the repository
git clone https://github.com/LarssonMartin1998/atlas.git
cd atlas

# Initialize submodules (including vcpkg)
git submodule update --init --recursive
```

### Step 2: Bootstrap vcpkg

**Linux/macOS:**
```bash
./setup-vcpkg.sh
```

**Windows:**
```bat
setup-vcpkg.bat
```

This will:
- Initialize the vcpkg submodule
- Bootstrap vcpkg for your platform
- Download and build required dependencies (taskflow, gtest)

### Step 3: Configure and Build

**Using CMake directly:**
```bash
# Configure
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -G Ninja

# Build
cmake --build build --parallel
```

**Alternative: Using Make wrapper (convenience):**
```bash
# This uses vcpkg automatically if available
make setup build
```

### Step 4: Run Tests

```bash
# Run tests
cd build && ctest --output-on-failure
```

## Troubleshooting

### C++23 Compiler Issues

If you encounter errors related to missing C++23 features:

1. **Verify compiler version:**
   ```bash
   clang++ --version  # Should be 18+ 
   g++ --version      # Should be 14+
   ```

2. **Check C++23 standard library support:**
   ```bash
   echo '#include <print>' | clang++ -std=c++23 -E -
   ```

3. **Consider using Nix:** For the most reliable build experience, especially for development, consider using the Nix environment:
   ```bash
   # If you have Nix installed
   nix develop
   make setup build test
   ```

### vcpkg Issues

1. **Clean vcpkg cache:**
   ```bash
   rm -rf vcpkg_installed/ build/
   ./setup-vcpkg.sh
   ```

2. **Update vcpkg:**
   ```bash
   cd vcpkg
   git pull
   cd ..
   ```

### Platform-Specific Issues

**Linux:**
- Ensure you have libc++ headers for Clang
- Install `libc++-dev` and `libc++abi-dev` if using Clang

**macOS:**
- Ensure Xcode Command Line Tools are up to date
- Use `brew install llvm` for latest LLVM if system version is too old

**Windows:**
- Use the "Developer Command Prompt for VS 2022" when building
- Ensure Windows SDK is installed with Visual Studio

## Integration with Game Projects

If you're using Atlas in your game project, add it as a submodule and include it in your CMakeLists.txt:

```cmake
# In your game's CMakeLists.txt
add_subdirectory(atlas)
target_link_libraries(your_game PRIVATE atlas::atlas)
```

Atlas automatically handles vcpkg integration when included as a subdirectory.

## Alternative: Docker Development Environment

For consistent builds across platforms, consider using the provided Docker setup:

```bash
# TODO: Docker setup will be added in future updates
```

---

**Note:** These instructions are maintained as a secondary build path. For the best development experience and guaranteed compatibility, we recommend using the Nix environment as described in the main README.md.