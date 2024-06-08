# Variables
BUILD_DIR = build
CMAKE = cmake
CMAKE_FLAGS = -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
CONAN = conan
CONAN_FLAGS = install . --output-folder=$(BUILD_DIR) --build=missing

# Default target
.PHONY: all
all: clean setup build

# Setup target
.PHONY: setup
setup:
	@echo "Setting up the build environment..."
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

# Clean target
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)
	@echo "Cleanup complete."
