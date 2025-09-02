/*
 * Example usage of ChiLow Independent Implementation
 * Demonstrates how to use the implementation as a library
 * 
 * Date: September 2025
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#define NO_MAIN  /* Prevent main function from being compiled */
#include "chilow.c"

#include <stdio.h>
#include <time.h>

void demo_basic_usage(void) {
    printf("ChiLow Library Usage Demo\n");
    printf("=========================\n\n");
    
    // Initialize the implementation
    chilow_init();
    
    // Example 1: Basic decryption
    printf("Example 1: Basic 32-bit decryption\n");
    uint32_t ciphertext = 0x12345678;
    uint64_t tweak = 0xABCDEF0123456789ULL;
    uint64_t key_hi = 0x123456789ABCDEFULL;
    uint64_t key_lo = 0xFEDCBA9876543210ULL;
    
    uint64_t result = chilow_decrypt_32bit(ciphertext, tweak, key_hi, key_lo);
    
    printf("  Ciphertext: 0x%08X\n", ciphertext);
    printf("  Tweak:      0x%016llX\n", (unsigned long long)tweak);
    printf("  Key:        0x%016llX%016llX\n", 
           (unsigned long long)key_hi, (unsigned long long)key_lo);
    printf("  Result:     0x%016llX\n", (unsigned long long)result);
    printf("\n");
    
    // Example 2: 40-bit variant
    printf("Example 2: 40-bit variant\n");
    uint64_t ciphertext40 = 0x123456789AULL;
    uint64_t result40 = chilow_decrypt_40bit(ciphertext40, tweak, key_hi, key_lo);
    
    printf("  Ciphertext: 0x%010llX\n", (unsigned long long)ciphertext40);
    printf("  Result:     0x%010llX\n", (unsigned long long)result40);
    printf("\n");
    
    // Example 3: Performance test
    printf("Example 3: Performance measurement\n");
    const int iterations = 100000;
    clock_t start = clock();
    
    for (int i = 0; i < iterations; i++) {
        chilow_decrypt_32bit(i, i * 2, i * 3, i * 4);
    }
    
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("  Iterations: %d\n", iterations);
    printf("  Time:       %.3f seconds\n", cpu_time);
    printf("  Rate:       %.0f ops/sec\n", iterations / cpu_time);
    printf("\n");
}

int main(void) {
    demo_basic_usage();
    return 0;
}
