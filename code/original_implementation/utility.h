uint64_t generateState64();

/*a_7,a_6.....,a_0*/
void byteToBits(uint8_t a, uint8_t *abin);
/*S_{n-1},.....,S_0*/
void printBitsToFile(const void *S, int nrofByte, FILE *fp);
void printBytesToFile(const void *S, int nrofByte, FILE *fp);
void printWordsToFile(const void *S, int nrofWord, FILE *fp);
void printBits(const void *S, int nrofByte);
void printBytes(const void *S, int nrofByte);
//Convention: WORD is 32 bits
void printWords(const void *S, int nrofWord);
void printAll(const void *S, int nrofByte);
void printBits128(const void *S, int nrofByte);
void printBytes128(const void *S, int nrofByte);
void printAll128(const void *S, int nrofByte);
void printMatrix(uint64_t *M, int size);
