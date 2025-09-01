/*
 * ChiLow Independent Implementation
 * 
 * A clean, unified implementation of the ChiLow cryptographic primitive
 * Following standard C programming practices with reduced semantic complexity
 * 
 * Author: Hosein Hadipour <hsn.hadipour@gmail.com>
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/*                              CONSTANTS & TYPES                            */
/* ========================================================================== */

#define NUM_ROUNDS 8

/* Bit masks for different sizes */
static const uint64_t BITMASK_15  = 0x7FFF;
static const uint64_t BITMASK_17  = 0x1FFFF;
static const uint64_t BITMASK_19  = 0x7FFFF;
static const uint64_t BITMASK_21  = 0x1FFFFF;
static const uint64_t BITMASK_31  = 0x7FFFFFFF;
static const uint64_t BITMASK_32  = 0xFFFFFFFF;
static const uint64_t BITMASK_33  = 0x1FFFFFFFF;
static const uint64_t BITMASK_40  = 0xFFFFFFFFFF;
static const uint64_t BITMASK_63  = 0x7FFFFFFFFFFFFFFF;

/* Round constants for different variants */
static const uint64_t ROUND_CONSTANTS[NUM_ROUNDS] = {
    0x0000001000000000UL, 0x0000002100000000UL, 0x0000004200000000UL, 0x0000008300000000UL,
    0x0000010400000000UL, 0x0000020500000000UL, 0x0000040600000000UL, 0x0000080700000000UL
};

static const uint64_t ROUND_CONSTANTS_40[NUM_ROUNDS] = {
    0x8000001000000000UL, 0x8000002100000000UL, 0x8000004200000000UL, 0x8000008300000000UL,
    0x8000010400000000UL, 0x8000020500000000UL, 0x8000040600000000UL, 0x8000080700000000UL
};

/* 128-bit integer structure */
typedef struct {
    uint64_t lo;  /* Lower 64 bits */
    uint64_t hi;  /* Upper 64 bits */
} uint128_t;

/* Linear transformation parameters */
typedef struct {
    int alpha[3];
    int beta[3];
} linear_params_t;

/* Linear layer matrices (global storage) */
static uint32_t linear_matrix_32_state[32];
static uint32_t linear_matrix_32_prf[32];
static uint64_t linear_matrix_40[40];
static uint64_t linear_matrix_64[64];
static uint128_t linear_matrix_128[128];

/* ========================================================================== */
/*                              UTILITY FUNCTIONS                            */
/* ========================================================================== */

/**
 * Calculate population count (number of set bits) for different sizes
 */
static inline int popcount32(uint32_t x) {
    return __builtin_popcount(x);
}

static inline int popcount64(uint64_t x) {
    return __builtin_popcountll(x);
}

/**
 * Circular rotation operations
 */
static inline uint64_t rotr64(uint64_t value, int shift, int width) {
    uint64_t mask = (1ULL << width) - 1;
    value &= mask;
    return ((value >> shift) | (value << (width - shift))) & mask;
}

/* ========================================================================== */
/*                              CHI OPERATIONS                               */
/* ========================================================================== */

/**
 * Basic chi operation: x ⊕ ((¬rot(x,1)) ∧ rot(x,2))
 * This is the core nonlinear transformation in ChiLow
 */
static uint64_t chi_transform(uint64_t input, uint64_t mask, int bit_width) {
    uint64_t rot1 = rotr64(input, 1, bit_width) & mask;
    uint64_t rot2 = rotr64(input, 2, bit_width) & mask;
    return (input ^ ((~rot1) & rot2)) & mask;
}

/**
 * ChiChi operation for variable-width inputs
 * Splits input into two parts and applies chi to each, with linear mixing
 */
static uint64_t chichi_transform(uint64_t input, uint64_t small_mask, uint64_t large_mask, int split_pos) {
    /* Split input into lower and upper parts */
    uint64_t lower_part = input & small_mask;
    uint64_t upper_part = (input >> (split_pos - 1)) & large_mask;
    
    /* Apply chi transformation to both parts */
    uint64_t transformed_lower = chi_transform(lower_part, small_mask, split_pos - 1);
    uint64_t transformed_upper = chi_transform(upper_part, large_mask, split_pos + 1);
    
    /* Linear mixing layer */
    uint64_t linear_mix = 0;
    linear_mix |= (((input >> split_pos) ^ (input >> (split_pos - 3))) & 1) << (split_pos - 3);
    linear_mix |= (((input >> (split_pos - 1)) ^ (input >> (split_pos - 2))) & 1) << (split_pos - 2);
    linear_mix |= (((input >> (split_pos - 3)) ^ (input >> (split_pos - 1)) ^ (input >> split_pos)) & 1) << (split_pos - 1);
    linear_mix |= (((input >> split_pos) ^ (input >> (split_pos - 2))) & 1) << split_pos;
    
    /* Combine results */
    return ((transformed_upper << (split_pos - 1)) | transformed_lower) ^ linear_mix;
}

/**
 * ChiChi operation for 128-bit values
 * More complex due to bit width spanning two 64-bit words
 */
static uint128_t chichi_transform_128(uint128_t input) {
    uint128_t result;
    
    /* Extract lower 63 bits and apply chi */
    uint64_t lower_63 = input.lo & BITMASK_63;
    uint64_t chi_lower = chi_transform(lower_63, BITMASK_63, 63);
    
    /* Handle 65-bit upper part with cross-word operations */
    uint64_t upper_65 = (input.hi << 1) | ((input.lo >> 63) & 1);
    uint64_t upper_msb = (input.hi >> 63) & 1;
    
    /* Perform rotations for 65-bit chi */
    uint64_t rot1_val, rot1_msb, rot2_val, rot2_msb;
    
    /* First rotation */
    rot1_val = (upper_msb << 63) | ((upper_65 >> 1) & BITMASK_63);
    rot1_msb = upper_65 & 1;
    
    /* Second rotation */
    rot2_val = (rot1_msb << 63) | ((rot1_val >> 1) & BITMASK_63);
    rot2_msb = rot1_val & 1;
    
    /* Apply chi to 65-bit value */
    uint64_t chi_upper_msb = (upper_msb ^ ((~rot1_msb) & rot2_msb)) & 1;
    uint64_t chi_upper_val = upper_65 ^ ((~rot1_val) & rot2_val);
    
    /* Reassemble with bit shuffling */
    chi_lower = (chi_lower & BITMASK_63) | ((chi_upper_val & 1) << 63);
    chi_upper_val = ((chi_upper_val >> 1) & BITMASK_63) | (chi_upper_msb << 63);
    
    /* Apply linear mixing for 128-bit case */
    uint64_t mix_lo = 0, mix_hi = 0;
    mix_lo |= (((input.hi & 1) ^ (input.lo >> 61)) & 1) << 61;
    mix_lo |= (((input.lo >> 63) ^ (input.lo >> 62)) & 1) << 62;
    mix_lo |= (((input.lo >> 61) ^ (input.lo >> 63) ^ (input.hi & 1)) & 1) << 63;
    mix_hi |= (((input.hi & 1) ^ (input.lo >> 62)) & 1);
    
    result.hi = chi_upper_val ^ mix_hi;
    result.lo = chi_lower ^ mix_lo;
    
    return result;
}

/* ========================================================================== */
/*                              LINEAR LAYER                                 */
/* ========================================================================== */

/**
 * Generate linear transformation matrix based on parameters
 * Each row has exactly 3 bits set according to the formula
 */
static void generate_linear_matrix_32(uint32_t* matrix, const linear_params_t* params) {
    for (int row = 0; row < 32; row++) {
        uint32_t matrix_row = 0;
        for (int term = 0; term < 3; term++) {
            int col = (params->alpha[term] * row + params->beta[term]) % 32;
            matrix_row |= (1U << col);
        }
        matrix[row] = matrix_row;
    }
}

static void generate_linear_matrix_40(uint64_t* matrix, const linear_params_t* params) {
    for (int row = 0; row < 40; row++) {
        uint64_t matrix_row = 0;
        for (int term = 0; term < 3; term++) {
            int col = (params->alpha[term] * row + params->beta[term]) % 40;
            matrix_row |= (1ULL << col);
        }
        matrix[row] = matrix_row;
    }
}

static void generate_linear_matrix_64(uint64_t* matrix, const linear_params_t* params) {
    for (int row = 0; row < 64; row++) {
        uint64_t matrix_row = 0;
        for (int term = 0; term < 3; term++) {
            int col = (params->alpha[term] * row + params->beta[term]) % 64;
            matrix_row |= (1ULL << col);
        }
        matrix[row] = matrix_row;
    }
}

static void generate_linear_matrix_128(uint128_t* matrix, const linear_params_t* params) {
    for (int row = 0; row < 128; row++) {
        uint128_t matrix_row = {0, 0};
        
        for (int term = 0; term < 3; term++) {
            int col = (params->alpha[term] * row + params->beta[term]) % 128;
            if (col < 64) {
                matrix_row.lo |= (1ULL << col);
            } else {
                matrix_row.hi |= (1ULL << (col - 64));
            }
        }
        matrix[row] = matrix_row;
    }
}

/**
 * Apply linear transformation using precomputed matrix
 */
static uint32_t apply_linear_32(uint32_t input, const uint32_t* matrix) {
    uint32_t output = 0;
    for (int bit = 0; bit < 32; bit++) {
        int parity = popcount32(matrix[bit] & input) & 1;
        output |= ((uint32_t)parity << bit);
    }
    return output;
}

static uint64_t apply_linear_40(uint64_t input, const uint64_t* matrix) {
    uint64_t output = 0;
    input &= BITMASK_40;
    for (int bit = 0; bit < 40; bit++) {
        int parity = popcount64(matrix[bit] & input) & 1;
        output |= ((uint64_t)parity << bit);
    }
    return output;
}

static uint64_t apply_linear_64(uint64_t input, const uint64_t* matrix) {
    uint64_t output = 0;
    for (int bit = 0; bit < 64; bit++) {
        int parity = popcount64(matrix[bit] & input) & 1;
        output |= ((uint64_t)parity << bit);
    }
    return output;
}

static uint128_t apply_linear_128(uint128_t input, const uint128_t* matrix) {
    uint128_t output = {0, 0};
    
    for (int bit = 0; bit < 64; bit++) {
        int parity = (popcount64(matrix[bit].lo & input.lo) ^ 
                     popcount64(matrix[bit].hi & input.hi)) & 1;
        output.lo |= ((uint64_t)parity << bit);
    }
    
    for (int bit = 64; bit < 128; bit++) {
        int parity = (popcount64(matrix[bit].lo & input.lo) ^ 
                     popcount64(matrix[bit].hi & input.hi)) & 1;
        output.hi |= ((uint64_t)parity << (bit - 64));
    }
    
    return output;
}

/* ========================================================================== */
/*                              INITIALIZATION                               */
/* ========================================================================== */

/**
 * Initialize all linear transformation matrices
 */
static void initialize_linear_matrices(void) {
    /* Parameters for different components */
    linear_params_t state_params = {{11, 11, 11}, {5, 9, 12}};
    linear_params_t prf_params = {{11, 11, 11}, {1, 26, 30}};
    linear_params_t state40_params = {{17, 17, 17}, {1, 9, 30}};
    linear_params_t tweak_params = {{3, 3, 3}, {1, 26, 50}};
    linear_params_t key_params = {{17, 17, 17}, {7, 11, 14}};
    
    generate_linear_matrix_32(linear_matrix_32_state, &state_params);
    generate_linear_matrix_32(linear_matrix_32_prf, &prf_params);
    generate_linear_matrix_40(linear_matrix_40, &state40_params);
    generate_linear_matrix_64(linear_matrix_64, &tweak_params);
    generate_linear_matrix_128(linear_matrix_128, &key_params);
}

/* ========================================================================== */
/*                              MAIN ALGORITHMS                              */
/* ========================================================================== */

/**
 * ChiLow decryption for 32-bit ciphertext
 */
static uint64_t chilow_decrypt_32(uint32_t ciphertext, uint64_t tweak, uint128_t key) {
    /* Initial whitening */
    uint32_t plaintext = ciphertext ^ (key.hi & BITMASK_32);
    uint32_t tag = ciphertext ^ ((key.hi >> 32) & BITMASK_32);
    tweak ^= key.lo;
    
    /* Round function iterations */
    for (int round = 0; round < NUM_ROUNDS - 1; round++) {
        /* Add round constant to key */
        key.hi ^= ROUND_CONSTANTS[round];
        
        /* Nonlinear layer */
        plaintext = chichi_transform(plaintext, BITMASK_15, BITMASK_17, 16);
        tag = chichi_transform(tag, BITMASK_15, BITMASK_17, 16);
        tweak = chichi_transform(tweak, BITMASK_31, BITMASK_33, 32);
        key = chichi_transform_128(key);
        
        /* Linear layer */
        plaintext = apply_linear_32(plaintext, linear_matrix_32_state);
        tag = apply_linear_32(tag, linear_matrix_32_prf);
        tweak = apply_linear_64(tweak, linear_matrix_64);
        key = apply_linear_128(key, linear_matrix_128);
        
        /* Interaction layer */
        plaintext ^= (tweak & BITMASK_32);
        tag ^= ((tweak >> 32) & BITMASK_32);
        tweak ^= key.lo;
    }
    
    /* Final round (simplified) */
    plaintext = chichi_transform(plaintext, BITMASK_15, BITMASK_17, 16);
    tag = chichi_transform(tag, BITMASK_15, BITMASK_17, 16);
    tweak = apply_linear_64(tweak, linear_matrix_64);
    plaintext ^= (tweak & BITMASK_32);
    tag ^= ((tweak >> 32) & BITMASK_32);
    
    return ((uint64_t)tag << 32) | (plaintext & BITMASK_32);
}

/**
 * ChiLow decryption with reduced number of rounds
 */
static uint64_t chilow_decrypt_32_reduced(uint32_t ciphertext, uint64_t tweak, uint128_t key, int num_rounds) {
    /* Initial whitening */
    uint32_t plaintext = ciphertext ^ (key.hi & BITMASK_32);
    uint32_t tag = ciphertext ^ ((key.hi >> 32) & BITMASK_32);
    tweak ^= key.lo;
    
    /* Round function iterations */
    for (int round = 0; round < num_rounds - 1; round++) {
        /* Add round constant to key */
        key.hi ^= ROUND_CONSTANTS[round];
        
        /* Nonlinear layer */
        plaintext = chichi_transform(plaintext, BITMASK_15, BITMASK_17, 16);
        tag = chichi_transform(tag, BITMASK_15, BITMASK_17, 16);
        tweak = chichi_transform(tweak, BITMASK_31, BITMASK_33, 32);
        key = chichi_transform_128(key);
        
        /* Linear layer */
        plaintext = apply_linear_32(plaintext, linear_matrix_32_state);
        tag = apply_linear_32(tag, linear_matrix_32_prf);
        tweak = apply_linear_64(tweak, linear_matrix_64);
        key = apply_linear_128(key, linear_matrix_128);
        
        /* Interaction layer */
        plaintext ^= (tweak & BITMASK_32);
        tag ^= ((tweak >> 32) & BITMASK_32);
        tweak ^= key.lo;
    }
    
    /* Final round (simplified) if we have at least one round */
    if (num_rounds > 0) {
        plaintext = chichi_transform(plaintext, BITMASK_15, BITMASK_17, 16);
        tag = chichi_transform(tag, BITMASK_15, BITMASK_17, 16);
        tweak = apply_linear_64(tweak, linear_matrix_64);
        plaintext ^= (tweak & BITMASK_32);
        tag ^= ((tweak >> 32) & BITMASK_32);
    }
    
    return ((uint64_t)tag << 32) | (plaintext & BITMASK_32);
}

/**
 * ChiLow decryption with reduced number of rounds, without final linear layer
 */
static uint64_t chilow_decrypt_32_half_reduced(uint32_t ciphertext, uint64_t tweak, uint128_t key, int num_rounds) {
    /* Initial whitening */
    uint32_t plaintext = ciphertext ^ (key.hi & BITMASK_32);
    uint32_t tag = ciphertext ^ ((key.hi >> 32) & BITMASK_32);
    tweak ^= key.lo;
    
    /* Round function iterations */
    for (int round = 0; round < num_rounds - 1; round++) {
        /* Add round constant to key */
        key.hi ^= ROUND_CONSTANTS[round];
        
        /* Nonlinear layer */
        plaintext = chichi_transform(plaintext, BITMASK_15, BITMASK_17, 16);
        tag = chichi_transform(tag, BITMASK_15, BITMASK_17, 16);
        tweak = chichi_transform(tweak, BITMASK_31, BITMASK_33, 32);
        key = chichi_transform_128(key);
        
        /* Linear layer */
        plaintext = apply_linear_32(plaintext, linear_matrix_32_state);
        tag = apply_linear_32(tag, linear_matrix_32_prf);
        tweak = apply_linear_64(tweak, linear_matrix_64);
        key = apply_linear_128(key, linear_matrix_128);
        
        /* Interaction layer */
        plaintext ^= (tweak & BITMASK_32);
        tag ^= ((tweak >> 32) & BITMASK_32);
        tweak ^= key.lo;
    }
    
    /* Final round without linear layer (only nonlinear + interaction) */
    if (num_rounds > 0) {
        plaintext = chichi_transform(plaintext, BITMASK_15, BITMASK_17, 16);
        tag = chichi_transform(tag, BITMASK_15, BITMASK_17, 16);
        /* Skip the linear layer on tweak */
        plaintext ^= (tweak & BITMASK_32);
        tag ^= ((tweak >> 32) & BITMASK_32);
    }
    
    return ((uint64_t)tag << 32) | (plaintext & BITMASK_32);
}

/**
 * ChiLow decryption for 40-bit ciphertext
 */
static uint64_t chilow_decrypt_40(uint64_t ciphertext, uint64_t tweak, uint128_t key) {
    /* Initial whitening */
    uint64_t plaintext = ciphertext ^ (key.hi & BITMASK_40);
    tweak ^= key.lo;
    
    /* Round function iterations */
    for (int round = 0; round < NUM_ROUNDS - 1; round++) {
        /* Add round constant to key */
        key.hi ^= ROUND_CONSTANTS_40[round];
        
        /* Nonlinear layer */
        plaintext = chichi_transform(plaintext, BITMASK_19, BITMASK_21, 20);
        tweak = chichi_transform(tweak, BITMASK_31, BITMASK_33, 32);
        key = chichi_transform_128(key);
        
        /* Linear layer */
        plaintext = apply_linear_40(plaintext, linear_matrix_40);
        tweak = apply_linear_64(tweak, linear_matrix_64);
        key = apply_linear_128(key, linear_matrix_128);
        
        /* Interaction layer */
        plaintext ^= (tweak & BITMASK_40);
        tweak ^= key.lo;
    }
    
    /* Final round (simplified) */
    plaintext = chichi_transform(plaintext, BITMASK_19, BITMASK_21, 20);
    tweak = apply_linear_64(tweak, linear_matrix_64);
    plaintext = (plaintext ^ (tweak & BITMASK_40)) & BITMASK_40;
    
    return plaintext;
}

/**
 * ChiLow decryption for 40-bit ciphertext with reduced number of rounds
 */
static uint64_t chilow_decrypt_40_reduced(uint64_t ciphertext, uint64_t tweak, uint128_t key, int num_rounds) {
    /* Initial whitening */
    uint64_t plaintext = ciphertext ^ (key.hi & BITMASK_40);
    tweak ^= key.lo;
    
    /* Round function iterations */
    for (int round = 0; round < num_rounds - 1; round++) {
        /* Add round constant to key */
        key.hi ^= ROUND_CONSTANTS_40[round];
        
        /* Nonlinear layer */
        plaintext = chichi_transform(plaintext, BITMASK_19, BITMASK_21, 20);
        tweak = chichi_transform(tweak, BITMASK_31, BITMASK_33, 32);
        key = chichi_transform_128(key);
        
        /* Linear layer */
        plaintext = apply_linear_40(plaintext, linear_matrix_40);
        tweak = apply_linear_64(tweak, linear_matrix_64);
        key = apply_linear_128(key, linear_matrix_128);
        
        /* Interaction layer */
        plaintext ^= (tweak & BITMASK_40);
        tweak ^= key.lo;
    }
    
    /* Final round (simplified) if we have at least one round */
    if (num_rounds > 0) {
        plaintext = chichi_transform(plaintext, BITMASK_19, BITMASK_21, 20);
        tweak = apply_linear_64(tweak, linear_matrix_64);
        plaintext = (plaintext ^ (tweak & BITMASK_40)) & BITMASK_40;
    }
    
    return plaintext;
}

/**
 * ChiLow decryption for 40-bit ciphertext with reduced number of rounds, without final linear layer
 */
static uint64_t chilow_decrypt_40_half_reduced(uint64_t ciphertext, uint64_t tweak, uint128_t key, int num_rounds) {
    /* Initial whitening */
    uint64_t plaintext = ciphertext ^ (key.hi & BITMASK_40);
    tweak ^= key.lo;
    
    /* Round function iterations */
    for (int round = 0; round < num_rounds - 1; round++) {
        /* Add round constant to key */
        key.hi ^= ROUND_CONSTANTS_40[round];
        
        /* Nonlinear layer */
        plaintext = chichi_transform(plaintext, BITMASK_19, BITMASK_21, 20);
        tweak = chichi_transform(tweak, BITMASK_31, BITMASK_33, 32);
        key = chichi_transform_128(key);
        
        /* Linear layer */
        plaintext = apply_linear_40(plaintext, linear_matrix_40);
        tweak = apply_linear_64(tweak, linear_matrix_64);
        key = apply_linear_128(key, linear_matrix_128);
        
        /* Interaction layer */
        plaintext ^= (tweak & BITMASK_40);
        tweak ^= key.lo;
    }
    
    /* Final round without linear layer (only nonlinear + interaction) */
    if (num_rounds > 0) {
        plaintext = chichi_transform(plaintext, BITMASK_19, BITMASK_21, 20);
        /* Skip the linear layer on tweak */
        plaintext = (plaintext ^ (tweak & BITMASK_40)) & BITMASK_40;
    }
    
    return plaintext;
}

/* ========================================================================== */
/*                              PUBLIC INTERFACE                             */
/* ========================================================================== */

/**
 * Initialize the ChiLow implementation
 * Must be called before using any decryption functions
 */
void chilow_init(void) {
    initialize_linear_matrices();
}

/**
 * ChiLow decryption - 32-bit variant
 */
uint64_t chilow_decrypt_32bit(uint32_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo) {
    uint128_t key = {key_lo, key_hi};
    return chilow_decrypt_32(ciphertext, tweak, key);
}

/**
 * ChiLow decryption - 40-bit variant  
 */
uint64_t chilow_decrypt_40bit(uint64_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo) {
    uint128_t key = {key_lo, key_hi};
    return chilow_decrypt_40(ciphertext, tweak, key);
}

/**
 * ChiLow decryption - 32-bit variant with reduced rounds
 */
uint64_t chilow_reduced_round_32bit(uint32_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo, int num_rounds) {
    uint128_t key = {key_lo, key_hi};
    return chilow_decrypt_32_reduced(ciphertext, tweak, key, num_rounds);
}

/**
 * ChiLow decryption - 40-bit variant with reduced rounds
 */
uint64_t chilow_reduced_round_40bit(uint64_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo, int num_rounds) {
    uint128_t key = {key_lo, key_hi};
    return chilow_decrypt_40_reduced(ciphertext, tweak, key, num_rounds);
}

/**
 * ChiLow decryption - 32-bit variant with reduced rounds, without final linear layer
 */
uint64_t chilow_half_reduced_round_32bit(uint32_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo, int num_rounds) {
    uint128_t key = {key_lo, key_hi};
    return chilow_decrypt_32_half_reduced(ciphertext, tweak, key, num_rounds);
}

/**
 * ChiLow decryption - 40-bit variant with reduced rounds, without final linear layer
 */
uint64_t chilow_half_reduced_round_40bit(uint64_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo, int num_rounds) {
    uint128_t key = {key_lo, key_hi};
    return chilow_decrypt_40_half_reduced(ciphertext, tweak, key, num_rounds);
}

/* ========================================================================== */
/*                              TEST VECTORS                                 */
/* ========================================================================== */

/**
 * Test the implementation with known vectors from ChiLow specification
 */
void chilow_test_vectors(void) {
    printf("ChiLow Implementation Test Vectors\n");
    printf("==================================\n\n");
    
    /* Test case 1: 32-bit from specification Table 6 */
    uint32_t c32 = 0x01234567;
    uint64_t t32 = 0x0011223344556677UL;
    uint64_t k1 = 0xFEDCBA9876543210UL;
    uint64_t k0 = 0x7766554433221100UL;
    
    uint64_t result32 = chilow_decrypt_32bit(c32, t32, k1, k0);
    
    printf("32-bit Test (ChiLow-(32+tau) Table 6):\n");
    printf("  Ciphertext: 0x%08X\n", c32);
    printf("  Tweak:      0x%016llX\n", (unsigned long long)t32);
    printf("  Key:        0x%016llX%016llX\n", (unsigned long long)k1, (unsigned long long)k0);
    printf("  Result:     0x%016llX\n", (unsigned long long)result32);
    printf("\n");
    
    /* Test case 2: 40-bit from specification Table 7 */
    uint64_t c40 = 0x317C83E4A7UL;
    uint64_t t40 = 0x0011223344556677UL;
    
    uint64_t result40 = chilow_decrypt_40bit(c40, t40, k1, k0);
    
    printf("40-bit Test (ChiLow-40 Table 7):\n");
    printf("  Ciphertext: 0x%010llX\n", (unsigned long long)c40);
    printf("  Tweak:      0x%016llX\n", (unsigned long long)t40);
    printf("  Key:        0x%016llX%016llX\n", (unsigned long long)k1, (unsigned long long)k0);
    printf("  Result:     0x%010llX\n", (unsigned long long)result40);
    printf("\n");
}

/* ========================================================================== */
/*                              MAIN FUNCTION                                */
/* ========================================================================== */

#ifndef NO_MAIN
int main(void) {
    printf("ChiLow Independent Implementation\n");
    printf("=================================\n\n");
    
    /* Initialize the implementation */
    chilow_init();
    
    /* Run test vectors */
    chilow_test_vectors();
    
    return 0;
}
#endif
