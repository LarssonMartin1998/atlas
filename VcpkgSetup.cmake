# VcpkgSetup.cmake - Non-Nix dependency management setup
# This module provides vcpkg integration for non-Nix users

# Detect and set up vcpkg if available (for non-Nix users)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# External library dependencies
function(setup_non_nix_dependencies)
    # No non-Nix specific dependencies currently
    # Core dependencies like Taskflow are handled by individual modules
endfunction()