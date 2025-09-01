uint64_t chi(uint64_t x, uint64_t mask, int size) {
    uint64_t x1 = ((x >> 1) | (x << (size - 1))) & mask;
    uint64_t x2 = ((x >> 2) | (x << (size - 2))) & mask;
    uint64_t y = (x ^ ((~x1) & x2)) & mask;
    return y;
}
uint64_t chichi(uint64_t x, uint64_t mask_small, uint64_t mask_big, int m) {
    uint64_t x0 = x & mask_small;
    uint64_t x1 = (x >> (m-1)) & mask_big;

    uint64_t y0 = chi(x0, mask_small, m-1);
    uint64_t y1 = chi(x1, mask_big  , m+1);

    uint64_t L = 0UL;
    L |= ( ((x >> (m  )) ^ (x >> (m-3))) & 0x01 ) << (m-3);
    L |= ( ((x >> (m-1)) ^ (x >> (m-2))) & 0x01 ) << (m-2);
    L |= ( ((x >> (m-3)) ^ (x >> (m-1)) ^ (x >> m)) & 0x01 ) << (m-1);
    L |= ( ((x >> (m  )) ^ (x >> (m-2))) & 0x01 ) << (m  );
    
    uint64_t y = ( (y1 << (m-1)) | y0 ) ^ L; 
    
    return y;
}

//Right rotate by 1 bit
void rotate65(uint64_t x_msb, uint64_t x, uint64_t *y_msb, uint64_t *y) {
    *y = (x_msb << 63) | ((x >> 1) & MASK63);
    *y_msb = x & 0x01UL;
}
U128 chichi128(U128 x) {
    int m = 64;
    U128 y;   
    uint64_t x0 = x.low & MASK63;
    uint64_t y0 = chi(x0, MASK63, m-1);

    //Applying chi65
    uint64_t x1 = (x.high << 1) | ((x.low >> 63) & 0x1);
    uint64_t x1_msb = (x.high >> 63) & 0x1UL;

    uint64_t a, a_msb, b, b_msb;
    rotate65(x1_msb, x1, &a_msb, &a);
    rotate65(a_msb, a, &b_msb, &b);

    uint64_t y1_msb = ( x1_msb ^ ((~a_msb) & b_msb) ) & 0x1UL;
    uint64_t y1 = ( x1 ^ ((~a) & b) );

    y0 = (y0 & MASK63) | ( (y1 & 0x01UL) << 63 );
    y1 = ((y1 >> 1) & MASK63) | (y1_msb << 63);
    
    uint64_t L0=0UL, L1 = 0UL;
    L0 |= ( ((x.high & 0x01UL) ^ (x.low >> (m-3))) & 0x01UL ) << (m-3); //61
    L0 |= ( ((x.low  >> (m-1)) ^ (x.low >> (m-2))) & 0x01UL ) << (m-2);    //62
    L0 |= ( ((x.low  >> (m-3)) ^ (x.low >> (m-1)) ^ (x.high & 0x01UL)) & 0x01UL ) << (m-1); //63
    L1 |= ( ((x.high & 0x01UL) ^ (x.low >> (m-2))) & 0x01UL ); //64
    
    y.high = y1 ^ L1;
    y.low  = y0 ^ L0;
    return y;
}

/* void testChiChi() { */
/*     printf("----------------------------------------------\n"); */
/*     uint32_t P = 0x76543210; */
/*     uint64_t T = 0xFEDCBA9876543210UL; */
/*     uint64_t K1 = 0xFEDCBA9876543210UL; */
/*     uint64_t K0 = 0xFEDCBA9876543210UL; */
/*     U128 K; K.high = K1; K.low = K0; */

/*     uint32_t PP = chichi(P, MASK15, MASK17, 16); */
/*     uint64_t TT = chichi(T, MASK31, MASK33, 32); */
/*     U128 KK = chichi128(K); */
/*     printBytes(&P, 4); */ 
/*     printBytes(&PP, 4); */ 
/*     printBytes(&T, 8); */ 
/*     printBytes(&TT, 8); */ 
/*     printBytes128(&K, 16); */ 
/*     printBytes128(&KK, 16); */ 
/*     printf("----------------------------------------------\n"); */
/* } */
