#!/usr/bin/env bash
# Bootstrap script for vcpkg on Unix systems (Linux/macOS)
set -e

echo "Setting up vcpkg for Atlas..."

# Detect architecture and provide helpful information
if [[ "$OSTYPE" == "darwin"* ]]; then
    ARCH=$(uname -m)
    echo "Detected macOS on $ARCH architecture"
    
    if [ "$ARCH" = "arm64" ]; then
        echo "✅ Apple Silicon (M1/M2) detected"
        echo "ℹ️  If you encounter vcpkg build issues, see BUILD.md troubleshooting section"
    else
        echo "ℹ️  Intel Mac detected"
    fi
fi

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
echo Read the BUILD.md for build instructions!
