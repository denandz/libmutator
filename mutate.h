#include "testcase.h"

#ifndef MUTATE_H
#define MUTATE_H

#define FLIP_BIT(_ar, _b) do { \
    uint8_t* _arf = (uint8_t*)(_ar); \
    uint32_t _bf = (_b); \
    _arf[_bf >> 3] ^= (1 << (_bf &7)); \
} while (0)

extern void mutate_init();
extern testcase_t * mutate(testcase_list_t * t);
extern testcase_t * mutate_testcase(testcase_t * t);
extern void mutate_set_fuzzfactor(int ff);
extern void mutate_bit(testcase_t * t);
extern void mutate_bit2(testcase_t * t);
extern void mutate_bit4(testcase_t * t);
extern void mutate_byte(testcase_t * t);
extern void mutate_random(testcase_t * t);
extern void mutate_insert_byte(testcase_t * t);
extern void mutate_insert_bytes(testcase_t * t);
extern void mutate_insert_ascii_bytes(testcase_t * t);
extern void mutate_shuffle(testcase_t * t);
extern void mutate_erase(testcase_t * t);
extern void mutate_inc(testcase_t * t);
extern void mutate_dec(testcase_t * t);
extern void mutate_not(testcase_t * t);

#endif
