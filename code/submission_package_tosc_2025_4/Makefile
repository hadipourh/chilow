# Makefile for ChiLow Independent Implementation
# Author: Hosein Hadipour <hsn.hadipour@gmail.com>
# Date: September 2025
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

# Compiler settings
CC = gcc
CFLAGS = -std=c99 -O3 -Wall -Wextra -Wpedantic -march=native
DEBUG_FLAGS = -g -DDEBUG -fsanitize=address,undefined
RELEASE_FLAGS = -DNDEBUG -flto

# Directories
BUILD_DIR = build
SRC_DIR = .

# Source files
SOURCES = chilow.c
TEST_SOURCES = test.c
EXAMPLE_SOURCES = example.c
INTEGRAL_SOURCES = integral.c
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_OBJECTS = $(TEST_SOURCES:%.c=$(BUILD_DIR)/%.o)
EXAMPLE_OBJECTS = $(EXAMPLE_SOURCES:%.c=$(BUILD_DIR)/%.o)
INTEGRAL_OBJECTS = $(INTEGRAL_SOURCES:%.c=$(BUILD_DIR)/%.o)
TARGET = chilow
TEST_TARGET = test
EXAMPLE_TARGET = example
INTEGRAL_TARGET = integral
DEBUG_TARGET = $(TARGET)_debug

# Default target
.PHONY: all
all: release

# Release build
.PHONY: release
release: CFLAGS += $(RELEASE_FLAGS)
release: $(BUILD_DIR)/$(TARGET)

# Debug build
.PHONY: debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(BUILD_DIR)/$(DEBUG_TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable (release)
$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# Link executable (debug)
$(BUILD_DIR)/$(DEBUG_TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# Link test executable
$(BUILD_DIR)/$(TEST_TARGET): $(TEST_OBJECTS) $(BUILD_DIR)/chilow_noMain.o
	$(CC) $(CFLAGS) $(TEST_OBJECTS) $(BUILD_DIR)/chilow_noMain.o -o $@

# Link example executable
$(BUILD_DIR)/$(EXAMPLE_TARGET): $(EXAMPLE_OBJECTS)
	$(CC) $(CFLAGS) $(EXAMPLE_OBJECTS) -o $@

# Link integral analysis executable
$(BUILD_DIR)/$(INTEGRAL_TARGET): $(INTEGRAL_OBJECTS)
	$(CC) $(CFLAGS) $(INTEGRAL_OBJECTS) -o $@

# Compile implementation without main for testing
$(BUILD_DIR)/chilow_noMain.o: chilow.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DNO_MAIN -c $< -o $@

# Run tests
.PHONY: test
test: release $(BUILD_DIR)/$(TEST_TARGET)
	@echo "Running ChiLow implementation tests..."
	./$(BUILD_DIR)/$(TEST_TARGET)

# Run basic implementation
.PHONY: run
run: release
	@echo "Running ChiLow basic implementation..."
	./$(BUILD_DIR)/$(TARGET)

# Run example
.PHONY: example
example: $(BUILD_DIR)/$(EXAMPLE_TARGET)
	@echo "Running ChiLow usage example..."
	./$(BUILD_DIR)/$(EXAMPLE_TARGET)

# Run original implementation for comparison
.PHONY: test-original
test-original:
	@echo "Building and running original implementation..."
	$(MAKE) -f Makefile clean || true
	$(MAKE) -f Makefile main
	@echo "Original implementation results:"
	./main

# Compare outputs
.PHONY: compare
compare: test test-original
	@echo "Comparison complete. Check outputs above."

# Performance test
.PHONY: benchmark
benchmark: release
	@echo "Running performance benchmark..."
	time ./$(BUILD_DIR)/$(TARGET)

# Memory check (requires valgrind - Linux only)
.PHONY: memcheck
memcheck: debug
	@echo "Running memory check..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --show-leak-kinds=all ./$(BUILD_DIR)/$(DEBUG_TARGET); \
	else \
		echo "valgrind not found or not supported on this platform."; \
		echo "Note: valgrind only works on Linux systems."; \
		echo "On macOS: Use Xcode Instruments or AddressSanitizer (built into debug builds)"; \
		echo "On Linux: sudo apt-get install valgrind"; \
		echo ""; \
		echo "Alternative: This debug build already includes AddressSanitizer:"; \
		echo "Running debug build with AddressSanitizer..."; \
		./$(BUILD_DIR)/$(DEBUG_TARGET); \
	fi

# Static analysis (requires cppcheck)
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c99 $(SOURCES); \
	else \
		echo "cppcheck not found. Please install cppcheck to run static analysis."; \
		echo "On macOS: brew install cppcheck"; \
		echo "On Ubuntu/Debian: sudo apt-get install cppcheck"; \
		exit 1; \
	fi

# Format code (requires clang-format)
.PHONY: format
format:
	@echo "Formatting code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i -style="{BasedOnStyle: LLVM, IndentWidth: 4, ColumnLimit: 100}" $(SOURCES); \
	else \
		echo "clang-format not found. Please install clang-format to format code."; \
		echo "On macOS: brew install clang-format"; \
		echo "On Ubuntu/Debian: sudo apt-get install clang-format"; \
		exit 1; \
	fi

# Integral cryptanalysis tool
.PHONY: integral
integral: $(BUILD_DIR)/$(INTEGRAL_TARGET)
	@echo "[*] Running integral cryptanalysis tool..."
	./$(BUILD_DIR)/$(INTEGRAL_TARGET)

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(DEBUG_TARGET)

# Clean everything including original build
.PHONY: distclean
distclean: clean
	$(MAKE) -f Makefile clean || true

# Show help
.PHONY: help
help:
	@echo "ChiLow Independent Implementation - Build System"
	@echo "==============================================="
	@echo ""
	@echo "Core targets:"
	@echo "  all         - Build release version (default)"
	@echo "  release     - Build optimized release version"
	@echo "  debug       - Build debug version with sanitizers"
	@echo "  test        - Run comprehensive test suite"
	@echo "  example     - Run usage examples"
	@echo "  integral    - Run integral cryptanalysis tool"
	@echo ""
	@echo "Development targets:"
	@echo "  benchmark   - Run performance benchmark"
	@echo "  memcheck    - Run memory check (uses AddressSanitizer on macOS, valgrind on Linux)"
	@echo "  analyze     - Run static analysis (requires cppcheck)"
	@echo "  format      - Format code (requires clang-format)"
	@echo ""
	@echo "Utility targets:"
	@echo "  clean       - Clean build artifacts"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Optional dependencies (install if needed):"
	@echo "  cppcheck     - brew install cppcheck (static analysis)"
	@echo "  clang-format - brew install clang-format (code formatting)"
	@echo "  valgrind     - Linux only (brew install not supported on macOS)"

# Dependencies
$(BUILD_DIR)/chilow.o: chilow.c
$(BUILD_DIR)/test.o: test.c
$(BUILD_DIR)/example.o: example.c

.PHONY: $(PHONY)
