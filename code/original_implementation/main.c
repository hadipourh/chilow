// gcc -Wall -Wextra kiki.c -o kiki
//gcc kiki.c testDifferential.c utility.c main.c -o  main -lm

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "kiki.h"  //chichi and linear layer are defined here
#include "utility.h"  //Some nice printing functions are defined here
void testProbability();

int main(){
    setLinearLayer();

    uint32_t C = 0x01234567;
    uint64_t C40 = 0x317C83E4A7;
    uint64_t T = 0x0011223344556677UL;
    // The 128 bit key is K = K1 << (64) || K0
    uint64_t K1 = 0xFEDCBA9876543210UL;
    uint64_t K0 = 0x7766554433221100UL;

    U128 K; K.high = K1; K.low =K0;
    uint64_t OUT = decryption(C, T, K, ROUNDS);
    printf("C= 0x%08X T= 0x%016lX K1||K0= 0x%016lX%016lX OUT= 0x%016lx \n", C, T, K.high, K.low, OUT);

    uint64_t OUT40 = decryption40(C40, T, K, ROUNDS);
    printf("C= 0x%016lX T= 0x%016lX K1||K0= 0x%016lX%016lX OUT= 0x%010lx \n", C40,T,K.high,K.low,OUT40);
}
