/*
 * ChiLow Independent Implementation - Test Suite
 * 
 * Comprehensive tests to verify the implementation against known test vectors
 * and compare with the original implementation
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Include our implementation */
extern void chilow_init(void);
extern uint64_t chilow_decrypt_32bit(uint32_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo);
extern uint64_t chilow_decrypt_40bit(uint64_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo);
extern uint64_t chilow_reduced_round_32bit(uint32_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo, int num_rounds);
extern uint64_t chilow_reduced_round_40bit(uint64_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo, int num_rounds);
extern uint64_t chilow_half_reduced_round_32bit(uint32_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo, int num_rounds);
extern uint64_t chilow_half_reduced_round_40bit(uint64_t ciphertext, uint64_t tweak, uint64_t key_hi, uint64_t key_lo, int num_rounds);

/* ========================================================================== */
/*                              TEST VECTORS                                 */
/* ========================================================================== */

typedef struct {
    uint32_t ciphertext;
    uint64_t tweak;
    uint64_t key_hi;
    uint64_t key_lo;
    uint64_t expected_result;
    const char* description;
} test_vector_32_t;

typedef struct {
    uint64_t ciphertext;
    uint64_t tweak;
    uint64_t key_hi;
    uint64_t key_lo;
    uint64_t expected_result;
    const char* description;
} test_vector_40_t;

/* Test vectors for 32-bit variant - from ChiLow specification Table 6 */
static const test_vector_32_t test_vectors_32[] = {
    {
        .ciphertext = 0x01234567,
        .tweak = 0x0011223344556677ULL,
        .key_hi = 0xFEDCBA9876543210ULL,
        .key_lo = 0x7766554433221100ULL,
        .expected_result = 0x0FBC7E642E75D127ULL,
        .description = "ChiLow-(32+tau) specification test vector (Table 6)"
    }
};

/* Test vectors for 40-bit variant - from ChiLow specification Table 7 */
static const test_vector_40_t test_vectors_40[] = {
    {
        .ciphertext = 0x317C83E4A7ULL,
        .tweak = 0x0011223344556677ULL,
        .key_hi = 0xFEDCBA9876543210ULL,
        .key_lo = 0x7766554433221100ULL,
        .expected_result = 0x0090545706ULL,
        .description = "ChiLow-40 specification test vector (Table 7)"
    }
};

/* ========================================================================== */
/*                              TEST FUNCTIONS                               */
/* ========================================================================== */

static int test_passed = 0;
static int test_failed = 0;

static void print_test_result(const char* test_name, int passed) {
    if (passed) {
        printf("[PASS] %s\n", test_name);
        test_passed++;
    } else {
        printf("[FAIL] %s\n", test_name);
        test_failed++;
    }
}

static void test_32bit_vectors(void) {
    printf("\n32-bit Test Vectors:\n");
    printf("====================\n");
    
    size_t num_tests = sizeof(test_vectors_32) / sizeof(test_vectors_32[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        const test_vector_32_t* tv = &test_vectors_32[i];
        
        printf("\nTest %zu: %s\n", i + 1, tv->description);
        printf("  Input:    C=0x%08X, T=0x%016llX\n", tv->ciphertext, 
               (unsigned long long)tv->tweak);
        printf("  Key:      0x%016llX%016llX\n", 
               (unsigned long long)tv->key_hi, (unsigned long long)tv->key_lo);
        
        uint64_t result = chilow_decrypt_32bit(tv->ciphertext, tv->tweak, 
                                              tv->key_hi, tv->key_lo);
        
        printf("  Result:   0x%016llX\n", (unsigned long long)result);
        printf("  Expected: 0x%016llX\n", (unsigned long long)tv->expected_result);
        
        int passed = (result == tv->expected_result);
        print_test_result(tv->description, passed);
    }
}

static void test_40bit_vectors(void) {
    printf("\n40-bit Test Vectors:\n");
    printf("====================\n");
    
    size_t num_tests = sizeof(test_vectors_40) / sizeof(test_vectors_40[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        const test_vector_40_t* tv = &test_vectors_40[i];
        
        printf("\nTest %zu: %s\n", i + 1, tv->description);
        printf("  Input:    C=0x%010llX, T=0x%016llX\n", 
               (unsigned long long)tv->ciphertext, (unsigned long long)tv->tweak);
        printf("  Key:      0x%016llX%016llX\n", 
               (unsigned long long)tv->key_hi, (unsigned long long)tv->key_lo);
        
        uint64_t result = chilow_decrypt_40bit(tv->ciphertext, tv->tweak, 
                                              tv->key_hi, tv->key_lo);
        
        printf("  Result:   0x%010llX\n", (unsigned long long)result);
        printf("  Expected: 0x%010llX\n", (unsigned long long)tv->expected_result);
        
        int passed = (result == tv->expected_result);
        print_test_result(tv->description, passed);
    }
}

static void test_edge_cases(void) {
    printf("\nBasic Functionality Tests:\n");
    printf("==========================\n");
    
    /* Test with all zeros */
    printf("\nAll zeros test:\n");
    uint64_t result_zeros = chilow_decrypt_32bit(0, 0, 0, 0);
    printf("  Input: all zeros -> 0x%016llX\n", (unsigned long long)result_zeros);
    
    /* Test with all ones (within 32-bit range) */
    printf("\nAll ones test:\n");
    uint64_t result_ones = chilow_decrypt_32bit(0xFFFFFFFF, 0xFFFFFFFFFFFFFFFFULL, 
                                               0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL);
    printf("  Input: all ones -> 0x%016llX\n", (unsigned long long)result_ones);
    
    /* Test a few simple patterns */
    printf("\nPattern tests:\n");
    uint64_t result1 = chilow_decrypt_32bit(0x12345678, 0x1234567890ABCDEFULL, 
                                           0xFEDCBA9876543210ULL, 0x0123456789ABCDEFULL);
    printf("  Pattern 1: -> 0x%016llX\n", (unsigned long long)result1);
    
    uint64_t result2 = chilow_decrypt_32bit(0xAAAAAAAA, 0x5555555555555555ULL,
                                           0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL);
    printf("  Pattern 2: -> 0x%016llX\n", (unsigned long long)result2);
}

static void performance_test(void) {
    printf("\nBasic Performance Test:\n");
    printf("=======================\n");
    
    const int num_iterations = 10000;
    
    printf("Running %d iterations...\n", num_iterations);
    
    /* Simple performance test */
    for (int i = 0; i < num_iterations; i++) {
        chilow_decrypt_32bit(i & 0xFFFFFFFF, i, i, i);
    }
    
    printf("Completed %d operations successfully.\n", num_iterations);
}

static void test_reduced_rounds(void) {
    printf("\nReduced Round Tests:\n");
    printf("====================\n");
    
    uint32_t c32 = 0x01234567;
    uint64_t c40 = 0x317C83E4A7ULL;
    uint64_t tweak = 0x0011223344556677ULL;
    uint64_t key_hi = 0xFEDCBA9876543210ULL;
    uint64_t key_lo = 0x7766554433221100ULL;
    
    printf("\n32-bit reduced round tests:\n");
    for (int rounds = 1; rounds <= 8; rounds++) {
        uint64_t result_reduced = chilow_reduced_round_32bit(c32, tweak, key_hi, key_lo, rounds);
        uint64_t result_half = chilow_half_reduced_round_32bit(c32, tweak, key_hi, key_lo, rounds);
        
        printf("  %d rounds: 0x%016llX (half: 0x%016llX)\n", 
               rounds, (unsigned long long)result_reduced, (unsigned long long)result_half);
    }
    
    printf("\n40-bit reduced round tests:\n");
    for (int rounds = 1; rounds <= 8; rounds++) {
        uint64_t result_reduced = chilow_reduced_round_40bit(c40, tweak, key_hi, key_lo, rounds);
        uint64_t result_half = chilow_half_reduced_round_40bit(c40, tweak, key_hi, key_lo, rounds);
        
        printf("  %d rounds: 0x%010llX (half: 0x%010llX)\n", 
               rounds, (unsigned long long)result_reduced, (unsigned long long)result_half);
    }
    
    /* Verify that 8 rounds gives the same result as the full function */
    uint64_t full_result_32 = chilow_decrypt_32bit(c32, tweak, key_hi, key_lo);
    uint64_t reduced_result_32 = chilow_reduced_round_32bit(c32, tweak, key_hi, key_lo, 8);
    
    uint64_t full_result_40 = chilow_decrypt_40bit(c40, tweak, key_hi, key_lo);
    uint64_t reduced_result_40 = chilow_reduced_round_40bit(c40, tweak, key_hi, key_lo, 8);
    
    printf("\nConsistency check:\n");
    printf("  32-bit: Full=0x%016llX, 8-round=0x%016llX %s\n", 
           (unsigned long long)full_result_32, (unsigned long long)reduced_result_32,
           (full_result_32 == reduced_result_32) ? "[PASS]" : "[FAIL]");
    printf("  40-bit: Full=0x%010llX, 8-round=0x%010llX %s\n", 
           (unsigned long long)full_result_40, (unsigned long long)reduced_result_40,
           (full_result_40 == reduced_result_40) ? "[PASS]" : "[FAIL]");
    
    if (full_result_32 == reduced_result_32 && full_result_40 == reduced_result_40) {
        test_passed++;
        printf("[PASS] Reduced round consistency test\n");
    } else {
        test_failed++;
        printf("[FAIL] Reduced round consistency test\n");
    }
}

/* ========================================================================== */
/*                              MAIN TEST RUNNER                             */
/* ========================================================================== */

int main(void) {
    printf("ChiLow Independent Implementation - Test Suite\n");
    printf("==============================================\n");
    
    /* Initialize the implementation */
    chilow_init();
    
    /* Run all tests */
    test_32bit_vectors();
    test_40bit_vectors();
    test_edge_cases();
    test_reduced_rounds();
    performance_test();
    
    /* Print summary */
    printf("\n");
    printf("Test Summary:\n");
    printf("=============\n");
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);
    printf("Total tests:  %d\n", test_passed + test_failed);
    
    if (test_failed == 0) {
        printf("\n[SUCCESS] All tests passed!\n");
        return 0;
    } else {
        printf("\n[ERROR] Some tests failed!\n");
        return 1;
    }
}
