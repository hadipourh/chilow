/*
 * ChiLow Integral Cryptanalysis Tool - Simplified Version
 * 
 * Copyright (C) 2025
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Include the main ChiLow implementation
#define NO_MAIN
#include "chilow.c"

/* ========================================================================== */
/*                              UTILITY FUNCTIONS                            */
/* ========================================================================== */

/**
 * Generate random 64-bit value
 */
static uint64_t random_uint64(void) {
    return ((uint64_t)rand() << 32) | rand();
}

/**
 * Set specific bit in a value
 */
static uint64_t set_bit(uint64_t value, int position, int bit_value) {
    if (bit_value) {
        return value | (1ULL << position);
    } else {
        return value & ~(1ULL << position);
    }
}

/**
 * Get specific bit from a value
 */
static int get_bit(uint64_t value, int position) {
    return (value >> position) & 1;
}

/**
 * Parse comma-separated list of integers
 * Returns number of integers parsed, fills the array
 */
static int parse_int_list(const char* str, int* array, int max_count) {
    int count = 0;
    char* str_copy = malloc(strlen(str) + 1);
    strcpy(str_copy, str);
    
    char* token = strtok(str_copy, ",");
    while (token != NULL && count < max_count) {
        array[count] = atoi(token);
        count++;
        token = strtok(NULL, ",");
    }
    
    free(str_copy);
    return count;
}

/**
 * Print array of bit positions
 */
static void print_bit_positions(const int* positions, int count, const char* name) {
    printf("%s positions: ", name);
    if (count == 0) {
        printf("none");
    } else {
        for (int i = 0; i < count; i++) {
            printf("%d", positions[i]);
            if (i < count - 1) printf(",");
        }
    }
    printf("\n");
}

/* ========================================================================== */
/*                              MAIN INTEGRAL TEST                           */
/* ========================================================================== */

/**
 * Test integral distinguisher with specified parameters
 * 
 * @param rounds Number of rounds to test (1-8)
 * @param active_positions Array of active bit positions in input
 * @param num_active Number of active bits
 * @param balanced_positions Array of balanced bit positions in output to check
 * @param num_balanced Number of balanced bits to check
 * @param repetitions Number of repetitions with random fixed parts
 * @param use_40bit 1 for 40-bit variant, 0 for 32-bit variant
 * @return Number of repetitions where ALL balanced bits were actually balanced
 */
static int test_integral_distinguisher(int rounds, const int* active_positions, int num_active,
                                     const int* balanced_positions, int num_balanced, 
                                     int repetitions, int use_40bit) {
    
    int successful_repetitions = 0;
    int total_inputs = 1 << num_active;  // 2^num_active inputs per set
    
    printf("\nIntegral Distinguisher Test\n");
    printf("===========================\n");
    printf("Variant: %s\n", use_40bit ? "40-bit ChiLow" : "32-bit ChiLow");
    printf("Rounds: %d\n", rounds);
    print_bit_positions(active_positions, num_active, "Active");
    print_bit_positions(balanced_positions, num_balanced, "Balanced");
    printf("Repetitions: %d\n", repetitions);
    printf("Inputs per set: %d\n", total_inputs);
    printf("\n");
    
    for (int rep = 0; rep < repetitions; rep++) {
        // Generate random values for fixed parts
        uint32_t base_ciphertext_32 = (uint32_t)random_uint64();
        uint64_t base_ciphertext_40 = random_uint64() & 0xFFFFFFFFFFULL;
        uint64_t base_tweak = random_uint64();
        uint64_t base_key_hi = random_uint64();
        uint64_t base_key_lo = random_uint64();
        
        // Clear active bit positions in base values
        for (int i = 0; i < num_active; i++) {
            if (use_40bit) {
                base_ciphertext_40 = set_bit(base_ciphertext_40, active_positions[i], 0);
            } else {
                base_ciphertext_32 = (uint32_t)set_bit(base_ciphertext_32, active_positions[i], 0);
            }
        }
        
        // Compute XOR sum over all possible active bit combinations
        uint64_t xor_sum = 0;
        
        for (int input = 0; input < total_inputs; input++) {
            uint32_t ciphertext_32 = base_ciphertext_32;
            uint64_t ciphertext_40 = base_ciphertext_40;
            
            // Set active bits according to input pattern
            for (int bit = 0; bit < num_active; bit++) {
                int bit_value = (input >> bit) & 1;
                if (use_40bit) {
                    ciphertext_40 = set_bit(ciphertext_40, active_positions[bit], bit_value);
                } else {
                    ciphertext_32 = (uint32_t)set_bit(ciphertext_32, active_positions[bit], bit_value);
                }
            }
            
            // Compute ChiLow result using complete rounds for integral cryptanalysis
            uint64_t result;
            if (use_40bit) {
                result = chilow_complete_rounds_40bit(ciphertext_40, base_tweak, 
                                                     base_key_hi, base_key_lo, rounds);
            } else {
                result = chilow_complete_rounds_32bit(ciphertext_32, base_tweak, 
                                                     base_key_hi, base_key_lo, rounds);
            }
            
            xor_sum ^= result;
        }
        
        // Check if all specified balanced bits are actually balanced (zero)
        int all_balanced = 1;
        int balanced_count = 0;
        
        for (int i = 0; i < num_balanced; i++) {
            int bit_pos = balanced_positions[i];
            int is_balanced = (get_bit(xor_sum, bit_pos) == 0);
            if (is_balanced) {
                balanced_count++;
            } else {
                all_balanced = 0;
            }
        }
        
        if (all_balanced) {
            successful_repetitions++;
        }
        
        // Print detailed results for first few repetitions
        if (rep < 5 || rep == repetitions - 1) {
            if (use_40bit) {
                printf("Repetition %d: XOR sum = 0x%010llX, Balanced bits: %d/%d",
                       rep + 1, (unsigned long long)xor_sum, balanced_count, num_balanced);
            } else {
                // For 32-bit variant, separate plaintext (lower 32) and tag (upper 32)
                uint32_t plaintext_xor = (uint32_t)(xor_sum & 0xFFFFFFFF);
                uint32_t tag_xor = (uint32_t)(xor_sum >> 32);
                printf("Repetition %d: Plaintext XOR = 0x%08X, Tag XOR = 0x%08X, Balanced bits: %d/%d",
                       rep + 1, plaintext_xor, tag_xor, balanced_count, num_balanced);
            }
            if (all_balanced) {
                printf(" [SUCCESS]");
            } else {
                printf(" [FAILED]");
            }
            printf("\n");
        } else if (rep == 5 && repetitions > 6) {
            printf("... (showing first 5 and last repetitions) ...\n");
        }
    }
    
    printf("\nResults Summary:\n");
    printf("Successful repetitions: %d/%d (%.1f%%)\n", 
           successful_repetitions, repetitions, 
           100.0 * successful_repetitions / repetitions);
    
    if (successful_repetitions == repetitions) {
        printf("*** INTEGRAL DISTINGUISHER CONFIRMED ***\n");
    } else if (successful_repetitions > repetitions * 0.8) {
        printf("*** STRONG INTEGRAL BIAS DETECTED ***\n");
    } else {
        printf("*** NO CLEAR INTEGRAL DISTINGUISHER ***\n");
    }
    
    return successful_repetitions;
}

/* ========================================================================== */
/*                                 MAIN PROGRAM                              */
/* ========================================================================== */

int main(int argc, char* argv[]) {
    // Initialize random seed
    srand((unsigned int)time(NULL));
    
    // Initialize ChiLow
    chilow_init();
    
    printf("ChiLow Integral Cryptanalysis Tool\n");
    printf("===================================\n");
    printf("License: GPL v3.0\n\n");
    
    int rounds, repetitions, use_40bit = 0;
    int active_positions[64], balanced_positions[64];
    int num_active, num_balanced;
    
    if (argc >= 5) {
        // Parse command line arguments
        rounds = atoi(argv[1]);
        
        num_active = parse_int_list(argv[2], active_positions, 64);
        num_balanced = parse_int_list(argv[3], balanced_positions, 64);
        repetitions = atoi(argv[4]);
        
        if (argc >= 6) {
            use_40bit = atoi(argv[5]);
        }
        
        // Validate inputs
        if (rounds < 1 || rounds > 8) {
            printf("Error: Rounds must be between 1 and 8\n");
            return 1;
        }
        if (num_active == 0) {
            printf("Error: Must specify at least one active bit\n");
            return 1;
        }
        if (num_balanced == 0) {
            printf("Error: Must specify at least one balanced bit to check\n");
            return 1;
        }
        if (repetitions < 1) {
            printf("Error: Repetitions must be at least 1\n");
            return 1;
        }
        
        test_integral_distinguisher(rounds, active_positions, num_active,
                                  balanced_positions, num_balanced, 
                                  repetitions, use_40bit);
    } else {
        if (argc == 1) {
            // Default test case
            printf("Running default test case...\n");
            
            rounds = 3;
            active_positions[0] = 21; active_positions[1] = 23; active_positions[2] = 25;
            num_active = 3;
            balanced_positions[0] = 2; balanced_positions[1] = 3; 
            balanced_positions[2] = 14; balanced_positions[3] = 25; balanced_positions[4] = 26;
            num_balanced = 5;
            repetitions = 10;
            use_40bit = 0;  // Use 32-bit variant
            
            test_integral_distinguisher(rounds, active_positions, num_active,
                                      balanced_positions, num_balanced, 
                                      repetitions, use_40bit);
        } else {
            // Show usage
            printf("Usage: %s <rounds> <active_bits> <balanced_bits> <repetitions> [use_40bit]\n", argv[0]);
            printf("  rounds:        Number of rounds (1-8)\n");
            printf("  active_bits:   Comma-separated list of active bit positions (e.g., \"0,1,2\")\n");
            printf("  balanced_bits: Comma-separated list of balanced bit positions (e.g., \"0,15,31\")\n");
            printf("  repetitions:   Number of repetitions with random fixed parts\n");
            printf("  use_40bit:     1 for 40-bit variant, 0 for 32-bit variant (optional, default 0)\n\n");
            
            printf("Bit Numbering Convention:\n");
            printf("  - Bit positions are counted from RIGHT to LEFT (LSB to MSB)\n");
            printf("  - Position 0 = rightmost bit (least significant)\n");
            printf("  - For 32-bit variant: positions 0-31 = plaintext, 32-63 = tag\n");
            printf("  - For 40-bit variant: positions 0-39 = output bits\n");
            printf("  - Example: 0x12345678 has bit 0=0, bit 1=0, bit 2=0, bit 3=1, etc.\n\n");
            
            printf("Examples:\n");
            printf("  %s 3 \"0,1\" \"0,15,30,31\" 10 0\n", argv[0]);
            printf("  %s 2 \"0\" \"31\" 100 1\n", argv[0]);
            printf("\nTo run with default parameters, use: %s\n", argv[0]);
            return 1;
        }
    }
    
    return 0;
}
