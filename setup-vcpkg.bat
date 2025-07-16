@echo off
REM Bootstrap script for vcpkg on Windows
setlocal

echo Setting up vcpkg for Atlas...

REM Initialize vcpkg submodule if not already done
if not exist "vcpkg\.vcpkg-root" (
    echo Initializing vcpkg submodule...
    git submodule update --init --recursive vcpkg
)

REM Bootstrap vcpkg
if not exist "vcpkg\vcpkg.exe" (
    echo Bootstrapping vcpkg...
    cd vcpkg
    call bootstrap-vcpkg.bat
    cd ..
)

echo vcpkg setup complete!
echo Read the BUILD.md for build instructions!
