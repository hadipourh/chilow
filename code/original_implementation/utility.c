#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "kiki.h"
#include "utility.h"

uint64_t generateState64(){
    uint64_t state1 = (uint64_t)(rand() & 0x00000000FFFFFFFFUL);
    uint64_t state2 = (uint64_t)(rand() & 0xFFFFFFFFUL);
    uint64_t state = (state1 << 32) | state2;
    return state;
}

/*a_7,a_6.....,a_0*/
void byteToBits(uint8_t a, uint8_t *abin) {
    int i;
    for(i=0; i<8; i++) {
        abin[i] = (a >> i) & 0x01;
    }
}
/*S_{n-1},.....,S_0*/
void printBitsToFile(const void *S, int nrofByte, FILE *fp) {
    int i, j;
    uint8_t *f = (uint8_t *)S;
    uint8_t bbin[8];
    for(i=nrofByte-1; i>=0; i--){
        byteToBits(f[i], bbin);
        for(j=7; j>=0; j--) {
            /* fprintf(fp, "%0d", bbin[j]); */
            if(bbin[j] == 1) {
                fprintf(fp, "1");
            }
            else {
                fprintf(fp, "-");
            }
        }
        /* fprintf(fp, " "); */
    }
    fprintf(fp, "\n");
}

void printBytesToFile(const void *S, int nrofByte, FILE *fp) {
    int i;
    uint8_t *f = (uint8_t *)S;
    fprintf(fp, "0x");
    for(i=nrofByte-1; i>=0; i--){
        fprintf(fp, "%02X", f[i]);
    }
    fprintf(fp, " ");
}

void printWordsToFile(const void *S, int nrofWord, FILE *fp) {
    int i;
    uint32_t *f = (uint32_t *)S;
    for(i=nrofWord-1; i>=0; i--){
        fprintf(fp, "%08X", f[i]);
    }
    fprintf(fp, "\n");
}

void printBits(const void *S, int nrofByte){
    printBitsToFile(S, nrofByte, stdout);
}

void printBytes(const void *S, int nrofByte){
    printBytesToFile(S, nrofByte, stdout);
}

//Convention: WORD is 32 bits
void printWords(const void *S, int nrofWord){
    printWordsToFile(S, nrofWord, stdout);
}

void printAll(const void *S, int nrofByte){
    printBits(S, nrofByte);
    printBytes(S, nrofByte);
    printWords(S, nrofByte/4);
}

void printBits128(const void *S, int nrofByte){
    printf("%d \n", nrofByte);
    nrofByte = nrofByte / 2;
    U128 *f = (U128 *)S;
    printBitsToFile(&(f->high), nrofByte, stdout);
    printBitsToFile(&(f->low), nrofByte, stdout);
}
void printBytesToFile1(const void *S, int nrofByte, FILE *fp) {
    int i;
    uint8_t *f = (uint8_t *)S;
    for(i=nrofByte-1; i>=0; i--){
        fprintf(fp, "%02X", f[i]);
    }
}
void printBytes128(const void *S, int nrofByte){
    nrofByte = nrofByte/2;
    U128 *f = (U128 *)S;
    fprintf(stdout, "0x");
    printBytesToFile1(&(f->high), nrofByte, stdout);
    printBytesToFile1(&(f->low), nrofByte, stdout);
}
void printAll128(const void *S, int nrofByte){
    printBits128(S, nrofByte);
    printBytes128(S, nrofByte);
}

uint64_t frombin(int *M, int size){
    int i;
    uint64_t y = 0x00UL;

    for(i=size-1;i>=0;i--){
        y = ((y << 1) | M[i]);
    }
    return y;
}
void tobin(int *bin, uint64_t row, int size){
    memset(bin, 0x00, 128*sizeof(int));
    int i;
    for(i=0;i<size;i++){
        bin[i] = (row >> i) & 0x01;
    }
}
void printMatrix(uint64_t *M, int size){
    int rowHW = 0;
    printf("-------------------------------------------------------------\n");
    for(int i=0; i<size; i++){
        int row[128];
        tobin(row, M[i], size);
        for(int j=0; j<size; j++){
            printf("%01d ", row[j]);
        }
        rowHW = __builtin_popcountll(M[i]);
        printf("| => %d \n", rowHW);
    }
    printf("------------------------------------------------------------\n");
}

