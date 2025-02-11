# Variables
BUILD_TESTS ?= ON
BUILD_TYPE ?= Debug
BUILD_DIR = build
CMAKE = cmake
CMAKE_GENERATOR ?= Ninja
CMAKE_FLAGS = -G $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_TESTS=$(BUILD_TESTS)
CTEST = ctest

# Default target
.PHONY: all # Don't run this as go-to during development, only intended as a CI target/clean build.
all: clean setup build analyze test

.DEFAULT_GOAL := build

# Generate cmake target
.PHONY: generate
generate:
	@echo "Generating the project..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_FLAGS) ..
	@echo "Generation complete."

# Setup target
.PHONY: setup
setup:
	@echo "Setting up the build environment..."
	@ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json
	@mkdir -p $(BUILD_DIR)
	@make generate
	@echo "Environment setup complete."

# Build target
.PHONY: build
build:
	@echo "Building the project..."
	echo $(nproc)
	@$(CMAKE) --build $(BUILD_DIR) -- -j $(shell nproc)
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
	@cd generated && fd --max-depth 1 --exclude 'CMakeLists.txt' --exclude 'template' . -X rm -rf --
	@echo "Cleanup complete."
