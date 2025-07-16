#!/usr/bin/env bash
# Bootstrap script for vcpkg on Unix systems (Linux/macOS)
set -e

echo "Setting up vcpkg for Atlas..."

# Initialize vcpkg submodule if not already done
if [ ! -f "vcpkg/.vcpkg-root" ]; then
    echo "Initializing vcpkg submodule..."
    git submodule update --init --recursive vcpkg
fi

# Bootstrap vcpkg
if [ ! -f "vcpkg/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    cd vcpkg
    ./bootstrap-vcpkg.sh
    cd ..
fi

echo "vcpkg setup complete!"
echo "You can now build the project using:"
echo "  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake"
echo "  cmake --build build"
