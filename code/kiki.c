#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PRINT

int alphaState[3] = {11, 11, 11};
int betaState [3] = { 5,  9, 12};
int alphaPrf  [3] = {11, 11, 11};
int betaPrf   [3] = { 1, 26, 30};
int alphaTweak[3] = { 3,  3,  3};
int betaTweak [3] = { 1, 26, 50};
int alphaKey  [3] = {17, 17, 17};
int betaKey   [3] = { 7, 11, 14};


int alphaState40[3] = {17, 17, 17};
int betaState40 [3] = { 1,  9, 30};
/* int alphaState40[3] = { 3,  3,  3}; */
/* int betaState40 [3] = { 1,  6, 13}; */

#include "kiki.h"     
#include "utility.h"  //Some nice printing functions are defined here

//Matrix are loaded here. Each row of a matrix is a 32, 64 or 128-bit unsigned integer
//Then if Y = M*X, the Y[i] = parity_of(M[i] & X[i])
//Matrix are loaded by setLinearLayer() defined in linearLayer.h
uint32_t M32_0[32]; 
uint32_t M32_1[32];
uint64_t M40[40]; 
uint64_t M64[64];
U128     M128[128];

#include "chichi.h"     //chichi functions are defined here
#include "linearLayer.h" //Linear Layers are defined here

void printState(char *text, uint32_t P, uint32_t tag, uint64_t T, U128 K) {
    /* printf("-------------------------------------------------\n"); */
    printf("%s :", text);
    printBytes(&P, 4);   
    printBytes(&tag, 4);   
    printBytes(&T, 8);   
    printBytes128(&K, 16);   
    printf("\n");
    /* printf("-------------------------------------------------\n"); */
}
void printState40(char *text, uint64_t P, uint64_t T, U128 K) {
    /* printf("-------------------------------------------------\n"); */
    printf("%s :", text);
    printBytes(&P, 5);   
    printBytes(&T, 8);   
    printBytes128(&K, 16);   
    printf("\n");
    /* printf("-------------------------------------------------\n"); */
}

uint64_t decryption(uint32_t C, uint64_t T, U128 K, int rounds) {
    uint64_t RC[8] = {0x0000001000000000UL, 0x0000002100000000UL, 0x0000004200000000UL, 0x0000008300000000UL, 0x0000010400000000UL, 0x0000020500000000UL, 0x0000040600000000UL, 0x0000080700000000UL};
    char text[256];

#ifdef PRINT
    printf("Input :");
    printBytes(&C, 4);
    printf(", Tweak :");
    printBytes(&T, 8);
    printf(", Key :");
    printBytes128(&K, 16);
    printf("\n");
#endif
#ifdef PRINT
    printState("Input", C, C, T, K);
#endif
    //For the Pre Whitining Key, we Add K.high, i.e., ||tag = (C||C) ^ K.high
    uint32_t p     = C ^ (K.high & MASK32);
    uint32_t tag   = C ^ ((K.high >> 32) & MASK32);
    T = T ^ K.low;
    
#ifdef PRINT
    printState("Key Whitening", p, tag, T, K);
#endif

    for (int i=0; i<rounds-1; i++) {
        K.high = K.high ^ RC[i];
        //Apply SBOX (chichi and chichi128 functions are defined in chichi.h)
        p   = chichi(p, MASK15, MASK17, 16);
        tag = chichi(tag, MASK15, MASK17, 16);
        T   = chichi(T, MASK31, MASK33, 32);
        K   = chichi128(K);
        
        //Apply Linear Layer (linear layer functions are defined in linearLayer.h)
        p   = linearLayer32(p  , M32_0);
        tag = linearLayer32(tag, M32_1);
        T   = linearLayer64(T, M64);
        K   = linearLayer128(K, M128);
        
        //Apply interaction layer
        p     = p ^ (T & MASK32);
        tag   = tag   ^ ((T >> 32) & MASK32);
        T     = T ^ K.low;

#ifdef PRINT
        sprintf(text, "Round %d", i);
        printState(text, p, tag, T, K);
#endif
    }
    //Last Round
    p   = chichi(p, MASK15, MASK17, 16);
    tag = chichi(tag, MASK15, MASK17, 16);
    T   = linearLayer64(T, M64);
    p     = p     ^ (T & MASK32);
    tag   = tag   ^ ((T >> 32) & MASK32);

#ifdef PRINT
    sprintf(text, "Round %d", rounds-1);
    printState(text, p, tag, T, K);
#endif

    uint64_t OUT = ((uint64_t)tag << 32) | (p & MASK32);
    return OUT;
}
uint64_t decryption40(uint64_t C, uint64_t T, U128 K, int rounds) {
    uint64_t RC[8] = {0x8000001000000000UL, 0x8000002100000000UL, 0x8000004200000000UL, 0x8000008300000000UL, 0x8000010400000000UL, 0x8000020500000000UL, 0x8000040600000000UL, 0x8000080700000000UL};
    
    char text[256];
#ifdef PRINT
    printf("Input :");
    printBytes(&C, 5);
    printf(", Tweak :");
    printBytes(&T, 8);
    printf(", Key :");
    printBytes128(&K, 16);
    printf("\n");
#endif

#ifdef PRINT
    printState40("Input", C, T, K);
#endif

    //For the Pre Whitining Key, we Add K.high, i.e., ||tag = (C||C) ^ K.high
    uint64_t p = C ^ (K.high & MASK40);
    T          = T ^ K.low;
    
#ifdef PRINT
    printState40("Key Whitening", p, T, K);
#endif

   
    for (int i=0; i<rounds-1; i++) {
        K.high = K.high ^ RC[i];
        //Apply SBOX (chichi and chichi128 functions are defined in chichi.h)
        p   = chichi(p, MASK19, MASK21, 20);
        T   = chichi(T, MASK31, MASK33, 32);
        K = chichi128(K);
        
        //Apply Linear Layer (linear layer functions are defined in linearLayer.h)
        p   = linearLayer40(p, M40);
        T   = linearLayer64(T, M64);
        K = linearLayer128(K, M128);
        
        //Apply interaction layer
        p = p ^ (T & MASK40);
        T = T ^ K.low;

#ifdef PRINT
    sprintf(text, "Round %d", i);
    printState40(text, p, T, K);
#endif

    }
    //Last Round
    p   = chichi(p, MASK19, MASK21, 20);
    T   = linearLayer64(T, M64);
    p   = (p  ^ (T & MASK40)) & MASK40;

#ifdef PRINT
    sprintf(text, "Round %d", rounds-1);
    printState40(text, p, T, K);
#endif

    return p;
}
void setLinearLayer() {
    generateMatrix32 (  M32_0,  alphaState,    betaState );
    generateMatrix32 (  M32_1,  alphaPrf,      betaPrf );
    generateMatrix40 (  M40,    alphaState40,  betaState40 );
    generateMatrix64 (  M64,    alphaTweak,    betaTweak );
    generateMatrix128(  M128,   alphaKey,      betaKey   );
}
