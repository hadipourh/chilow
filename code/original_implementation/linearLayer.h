/*---------------------------Efficient Linear Layer-----------------------------*/
void generateMatrix32(uint32_t *M, int alpha[], int beta[]) {
    uint32_t y = 0UL;
    uint32_t size = 32;
    uint32_t i, i0, i1, i2;
    for(i=0; i<size; i++) {
        i0 = (alpha[0]*i + beta[0]) % size;
        i1 = (alpha[1]*i + beta[1]) % size;
        i2 = (alpha[2]*i + beta[2]) % size;
        y = (1UL << i0) | (1UL << i1) | (1UL << i2);
        M[i] = y;
    }
}
void generateMatrix64(uint64_t *M, int alpha[], int beta[]) {
    uint64_t y = 0UL;
    uint32_t size = 64;
    uint32_t i, i0, i1, i2;
    for(i=0; i<size; i++) {
        i0 = (alpha[0]*i + beta[0]) % size;
        i1 = (alpha[1]*i + beta[1]) % size;
        i2 = (alpha[2]*i + beta[2]) % size;
        y = (1UL << i0) | (1UL << i1) | (1UL << i2);
        M[i] = y;
    }
}
void generateMatrix40(uint64_t *M, int alpha[], int beta[]) {
    uint64_t y = 0UL;
    uint32_t size = 40;
    uint32_t i, i0, i1, i2;
    for(i=0; i<size; i++) {
        i0 = (alpha[0]*i + beta[0]) % size;
        i1 = (alpha[1]*i + beta[1]) % size;
        i2 = (alpha[2]*i + beta[2]) % size;
        y = (1UL << i0) | (1UL << i1) | (1UL << i2);
        M[i] = y;
    }
}
void generateMatrix128(U128 *M, int alpha[], int beta[]) {
    uint32_t size = 128;
    uint32_t i, i0, i1, i2;
    uint64_t y0 = 0UL, y1 = 0UL;
    for(i=0; i<size; i++) {
        i0 = (alpha[0]*i + beta[0]) % size;
        i1 = (alpha[1]*i + beta[1]) % size;
        i2 = (alpha[2]*i + beta[2]) % size;
        y0 = 0; y1 = 0; 
    
        if (i0 < 64) 
            y0 |= (1UL << i0); 
        else
            y1 |= (1UL << (i0-64)); 

        if (i1 < 64) 
            y0 |= (1UL << i1); 
        else
            y1 |= (1UL << (i1-64)); 

        if (i2 < 64) 
            y0 |= (1UL << i2); 
        else
            y1 |= (1UL << (i2-64));

        M[i].low  = y0;
        M[i].high = y1;
    }
}
uint32_t linearLayer32(uint32_t x, uint32_t *M) {
    uint32_t y = 0;
    for(int i=0; i<32; i++) {
        uint32_t b = __builtin_parity(M[i] & x);
        y = y | (b << i);
    }
    return y;
}
uint64_t linearLayer40(uint64_t x, uint64_t *M) {
    uint64_t y = 0;
    for(int i=0; i<40; i++) {
        uint64_t b = __builtin_parityl( (M[i] & x) & MASK40 );
        y = y | (b << i);
    }
    return y;
}
uint64_t linearLayer64(uint64_t x, uint64_t *M) {
    uint64_t y = 0;
    for(int i=0; i<64; i++) {
        uint64_t b = __builtin_parityl(M[i] & x);
        y = y | (b << i);
    }
    return y;
}
U128 linearLayer128(U128 x, U128 *M) {
    U128 y;
    y.low = y.high = 0;
    uint64_t b;
    for(int i=0; i<64; i++) {
        b = ((__builtin_parityl(M[i].low & x.low)) ^ (__builtin_parityl(M[i].high & x.high))) & 0x01;
        y.low |= (b << i);
    }
    for(int i=64; i<128; i++) {
        b = ((__builtin_parityl(M[i].low & x.low) ) ^ (__builtin_parityl(M[i].high & x.high))) & 0x01;
        y.high |= (b << (i-64));
    }
    return y;
}

/*---------------------------------------------------------------*/

