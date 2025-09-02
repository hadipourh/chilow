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
    time_t seconds_start,seconds_stop;
    printf("test3\n");

    U128 K; K.high = K1; K.low =K0;
    uint64_t OUT;// = decryption(C, T, K, 3);
    //printf("C= 0x%08X T= 0x%016lX K1||K0= 0x%016lX%016lX OUT= 0x%016lx \n", C, T, K.high, K.low, OUT);
    //OUT = 0;
    //time(&seconds_start);
    uint32_t rnd = rand();
    uint32_t cnt=0;
    uint32_t exp;
    srand(time(NULL));
    for(exp=0;exp<256;exp++)
    {
	    
	    rnd = rand();
	    printf("rand=0x%08X\n",rnd);
	    OUT = 0;
	    for(uint32_t cnt=0;cnt<(1<<3);cnt++)
	    {
	    	//C=((cnt&1)<<2)^0xff0aff00^((cnt&2)<<3)^((cnt&4)<<27)^((cnt&8)<<28);//2,4,29,31
	    	//C=((cnt&1)<<5)^0xff0eff00^((cnt&2)<<6)^((cnt&4)<<7)^((cnt&8)<<8);//5,7,9,11
	    	C=((cnt&1)<<21)^rnd^((cnt&2)<<22)^((cnt&4)<<23);//21,23,25
	    	OUT ^= decryption(C, T, K, 3);
	    	//OUT ^= decryption((cnt&1)^((cnt&2)<<5)^((cnt&4)<<10)^((cnt&8)<<15)^((cnt&16)<<20), (T), K, 3);
	    	//printf("cnt=0x%x-------------------------------------------------\n",(cnt<<1));
	    }
	    //printf("C= 0x%08X T= 0x%016lX K1||K0= 0x%016lX%016lX OUT= 0x%016lx \n", C, T, K.high, K.low, (OUT&0x600400C));
	    if((OUT&0x600400C)!=0)
	    {
	    	printf("------------------------------------Test failed------------------------------------\n");
	    	cnt++;
	    	}
	}
	printf("test passed %i times\n",exp-cnt);
    //uint64_t OUT40 = decryption40(C40, T, K, ROUNDS);
   //printf("C= 0x%016lX T= 0x%016lX K1||K0= 0x%016lX%016lX OUT= 0x%010lx \n", C40,T,K.high,K.low,OUT40);
}
