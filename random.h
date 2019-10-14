#include <stdint.h>
#ifndef RANDOM_H
#define RANDOM_H

extern uint64_t random_next(void);
extern void init_seed();
void manual_seed(unsigned int seed);
extern uint64_t rand_next(void);
extern uint64_t rand_at_most(uint64_t n);
extern uint64_t rand_between(uint64_t min, uint64_t max);
extern void rand_buf(uint8_t * buf, uint64_t size);

#endif