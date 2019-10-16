#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "util.h"

// xoroshiro128+
static __thread pthread_once_t r_tonce = PTHREAD_ONCE_INIT;
static __thread uint64_t r_state[2];
static int seeded = 0;

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

void init_seed(){
    lm_debug("seeding from /dev/urandom\n");
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        fatal("Couldn't open /dev/urandom for reading");
    }
    if (read(fd, (uint8_t*)r_state, sizeof(r_state)) != sizeof(r_state)) {
        fatal("Couldn't read '%zu' bytes from /dev/urandom", sizeof(r_state));
    }
    close(fd);
   // printf("[!] Seeded\n");
}

void manual_seed(unsigned int seed){
    lm_debug("manually seeding with %u\n", seed);
    srand(seed);
    int r1 = rand();
    int r2 = rand();
    lm_debug("rand values %d %d\n", r1, r2);
    r_state[0] = r1;
    r_state[1] = r2;
    seeded = 1;
}

uint64_t rand_next(void) {
    if(seeded == 0){
        pthread_once(&r_tonce, init_seed);
    }

    const uint64_t s0 = r_state[0];
    uint64_t s1 = r_state[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    r_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
    r_state[1] = rotl(s1, 36); // c

    return result;
}

// returns a random number between 0 and n
uint64_t rand_at_most(uint64_t n){
    if(n == 0){
        return 0;
    }
    return rand_next() % (n+1); 
}

// returns a random number between min and max
uint64_t rand_between(uint64_t min, uint64_t max){
    if(min > max){
        fatal("Min > Max");
    }
    if(max == UINT64_MAX){
        return rand_next();
    }
    return rand_at_most(max - min) + min;
}

// populates a buffer with random bytes
void rand_buf(uint8_t * buf, uint64_t size){
    uint64_t i;

    if(size == 0){  
        fatal("rand_buf called with size == 0");
    }

    for(i = 0; i < size; i++){
        buf[i] = (uint8_t)rand_next();
    }
}
