# Variables
BUILD_TESTS ?= ON
BUILD_TYPE ?= Debug
BUILD_DIR = build
CMAKE = cmake
CMAKE_FLAGS = -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_TESTS=$(BUILD_TESTS)
CONAN = conan
CONAN_FLAGS = install . --output-folder=$(BUILD_DIR) --build=missing -s build_type=$(BUILD_TYPE)
CTEST = ctest

# Default target
.PHONY: all # Don't run this as go-to during development, only intended as a CI target/clean build.
all: clean setup build analyze test

.DEFAULT_GOAL := build

# Setup target
.PHONY: setup
setup:
	@echo "Setting up the build environment..."
	@ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json
	@mkdir -p $(BUILD_DIR)
	@$(CONAN) $(CONAN_FLAGS)
	@cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_FLAGS) ..
	@echo "Environment setup complete."

# Build target
.PHONY: build
build:
	@echo "Building the project..."
	@$(CMAKE) --build $(BUILD_DIR)
	@echo "Build complete."

# Test target
.PHONY: test
test:
	@echo "Running tests..."
	@cd $(BUILD_DIR) && $(CTEST) --output-on-failure
	@echo "Tests complete."

# Analyze target
.PHONY: analyze
analyze:
	@echo "Analyzing the project..."
	@scan-build $(MAKE) build
	@echo "Analysis complete."

# Clean target
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) CMakeUserPresets.json .cache
	@echo "Cleanup complete."
