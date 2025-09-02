/* #define PRINT */
#define PRINT1
#define MASK32 0xFFFFFFFF
#define MASK40 0xFFFFFFFFFFUL
#define MASK48 0xFFFFFFFFFFFFUL
#define MASK64 0xFFFFFFFFFFFFFFFFUL

#define MASK15 0x7FFF
#define MASK17 0x1FFFF
#define MASK19 0x7FFFFUL
#define MASK21 0x1FFFFFUL
#define MASK31 0x7FFFFFFF
#define MASK33 0x1FFFFFFFFUL
#define MASK63 0x7FFFFFFFFFFFFFFFUL

#define ROUNDS 8
struct U128 {
    uint64_t high;
    uint64_t low;
};
typedef struct U128 U128;
/* void printState(uint32_t tag, uint32_t P, uint64_t T, U128 K); */
void setLinearLayer();

void generateMatrix32(uint32_t *M, int alpha[], int beta[]);
void generateMatrix64(uint64_t *M, int alpha[], int beta[]);
void generateMatrix128(U128 *M,    int alpha[], int beta[]);

uint64_t decryption(uint32_t C, uint64_t T, U128 K, int rounds);
uint64_t decryption40(uint64_t C, uint64_t T, U128 K, int rounds);
uint64_t chi(uint64_t x, uint64_t mask, int size);
uint64_t chichi(uint64_t x, uint64_t mask_small, uint64_t mask_big, int m);
//Right rotate by 1 bit
void rotate65(uint64_t x_msb, uint64_t x, uint64_t *y_msb, uint64_t *y);
U128 chichi128(U128 x);
uint32_t linearLayer32(uint32_t x, uint32_t *M);
uint64_t linearLayer64(uint64_t x, uint64_t *M);
U128     linearLayer128(U128 x, U128 *M);
