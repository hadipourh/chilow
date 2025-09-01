# ChiLow Independent Implementation

A complete, independent implementation of the ChiLow cryptographic primitive with comprehensive testing and integral cryptanalysis tools.

**Author:** Hosein Hadipour <hsn.hadipour@gmail.com>  
**License:** GNU General Public License v3.0  
**Date:** September 2025

## Overview

ChiLow is a cryptographic primitive that provides efficient encryption and decryption operations. 
This implementation provides the implementation with enhanced features for cryptanalytic research.

### Supported Variants

* **32 bit ChiLow** with 32 bit ciphertext input and 64 bit output (32 bit plaintext + 32 bit tag)
* **40 bit ChiLow** with 40 bit ciphertext input and 40 bit output
* **Configurable rounds** from 1 to 8 rounds for cryptanalysis
* **Half reduced rounds** without final linear layer application

## Building and Installation

### Requirements

* C99 compatible compiler (GCC or Clang)
* Make utility

### Optional Development Tools

* **cppcheck** → Static code analysis (`brew install cppcheck`) 
* **clang-format** → Code formatting (`brew install clang-format`)
* **valgrind** → Memory leak detection (Linux only, not available on macOS)

*Note: On macOS, the debug build includes AddressSanitizer for memory checking. The Makefile will provide helpful installation instructions if tools are missing.*

### Quick Start

```bash
# Build the implementation
make release

# Run specification test vectors
make test

# Run integral cryptanalysis tool
make integral
```

### Available Build Targets

**Core Targets:**
* `release` → Optimized release build with link time optimization
* `debug` → Debug build with address sanitizer and undefined behavior sanitizer
* `test` → Run comprehensive test suite with all specification vectors
* `example` → Build and run usage examples
* `integral` → Build and run integral cryptanalysis tool

**Development Targets:**
* `benchmark` → Performance measurement and optimization verification
* `memcheck` → Memory checking (AddressSanitizer on macOS, valgrind on Linux)
* `analyze` → Static code analysis (requires cppcheck)
* `format` → Code formatting (requires clang-format)

**Utility Targets:**
* `clean` → Remove all build artifacts
* `help` → Display all available targets with dependency information

## Usage

### Basic API

The implementation provides a simple API for both variants:

```c
#include "chilow.c"

int main(void) {
    // Initialize the implementation
    chilow_init();
    
    // 32 bit ChiLow decryption
    uint32_t ciphertext = 0x01234567;
    uint64_t tweak = 0x0011223344556677ULL;
    uint64_t key_hi = 0xFEDCBA9876543210ULL;
    uint64_t key_lo = 0x7766554433221100ULL;
    
    uint64_t result = chilow_decrypt_32bit(ciphertext, tweak, key_hi, key_lo);
    
    // Extract plaintext and tag from result
    uint32_t plaintext = (uint32_t)(result & 0xFFFFFFFF);
    uint32_t tag = (uint32_t)(result >> 32);
    
    return 0;
}
```

### Reduced Round Analysis

For cryptanalytic research, the implementation provides functions with configurable round numbers:

```c
// Standard 8 rounds
uint64_t result = chilow_decrypt_32bit(ciphertext, tweak, key_hi, key_lo);

// Reduced to 5 rounds
uint64_t result = chilow_reduced_round_32bit(ciphertext, tweak, key_hi, key_lo, 5);

// Half reduced (5 rounds without final linear layer)
uint64_t result = chilow_half_reduced_round_32bit(ciphertext, tweak, key_hi, key_lo, 5);
```

## Integral Cryptanalysis Tool

The implementation includes a specialized tool for integral cryptanalysis with the following features:

### Command Line Interface

```bash
# Basic usage with default parameters
./integral

# Custom analysis
./integral <rounds> <active_bits> <balanced_bits> <repetitions> [use_40bit]

# Example: 3 rounds, active bits 0,1, check balance in bits 0,15,30,31
./integral 3 "0,1" "0,15,30,31" 10 0
```

### Input Parameters

* **rounds** → Number of ChiLow rounds to analyze (1 to 8)
* **active_bits** → Comma separated list of input bit positions that vary
* **balanced_bits** → Comma separated list of output bit positions to check for zero sum
* **repetitions** → Number of tests with different random fixed parts
* **use_40bit** → Use 40 bit variant (1) or 32 bit variant (0, default)

### Bit Position Reference

For the 32 bit variant:
* Positions 0 to 31 refer to plaintext bits (lower 32 bits of output)
* Positions 32 to 63 refer to tag bits (upper 32 bits of output)

For the 40 bit variant:
* Positions 0 to 39 refer to output bits

Bit numbering follows standard convention with position 0 as the least significant bit.

### Example Analysis Results

```
Repetition 1: Plaintext XOR = 0x00000000, Tag XOR = 0x00001004, Balanced bits: 32/32 [SUCCESS]
Repetition 2: Plaintext XOR = 0x00000000, Tag XOR = 0x00001000, Balanced bits: 32/32 [SUCCESS]

Results Summary:
Successful repetitions: 2/2 (100.0%)
*** INTEGRAL DISTINGUISHER CONFIRMED ***
```

This shows a perfect integral distinguisher for the plaintext part with the specified active bit configuration.

## Test Vectors

The implementation passes all official specification test vectors:

### 32 bit ChiLow (Table 6)
* **Input:** C=0x01234567, T=0x0011223344556677, K=0xFEDCBA98765432107766554433221100
* **Expected:** 0x0FBC7E642E75D127
* **Status:** PASS

### 40 bit ChiLow (Table 7)
* **Input:** C=0x317C83E4A7, T=0x0011223344556677, K=0xFEDCBA98765432107766554433221100
* **Expected:** 0x0090545706
* **Status:** PASS

All test vectors are taken directly from the official ChiLow specification document.

## Implementation Details

### Architecture

The implementation uses a unified single file design that includes:

* **128 bit arithmetic** using struct based approach for key operations
* **Optimized linear algebra** over GF(2) for matrix operations
* **Efficient nonlinear transformations** (Chi and ChiChi functions)
* **Constant time operations** for cryptographic security
* **Comprehensive error checking** with proper input validation

### Performance Characteristics

The implementation is optimized for:
* **High throughput** with native architecture targeting
* **Low memory usage** with minimal dynamic allocation
* **Cache efficiency** through data structure layout optimization
* **Compiler optimization** support with link time optimization

### Security Considerations

* **Constant time implementation** to prevent timing attacks
* **Proper memory management** with no memory leaks
* **Input validation** for all public API functions
* **Secure random number generation** for cryptanalysis tools

## File Structure

```
chilow.c                Main implementation (single unified file)
test.c                  Comprehensive test suite
example.c               Usage examples and demonstrations
integral.c              Integral cryptanalysis tool
Makefile                Professional build system
README.md               This documentation file
```

## Development and Testing

### Verification Process

All changes undergo thorough verification:

1. Specification test vector validation
2. Reduced round consistency checking
3. Memory safety analysis
4. Performance regression testing
5. Cross platform compatibility verification

## Contributing

This implementation follows standard C programming practices and welcomes contributions that maintain code quality and compatibility with the ChiLow specification.

### Guidelines

* Follow C99 standard practices
* Maintain zero compiler warnings
* Include comprehensive tests for new features
* Update documentation for API changes
* Ensure backward compatibility

## License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

## Acknowledgments

This implementation is based on the ChiLow specification and provides an independent, research oriented implementation suitable for cryptanalytic studies and practical applications.
