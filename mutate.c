#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "mutate.h"
#include "random.h"
#include "testcase.h"
#include "util.h"

extern int errno;
int fuzzfactor = 5;

void (* mutation_functions[])(testcase_t *) = {
    mutate_bit,
    mutate_bit2,
    mutate_bit4,
    mutate_byte,
    mutate_random,
    mutate_insert_byte,
    mutate_insert_bytes,
    mutate_insert_ascii_bytes,
    mutate_shuffle,
    mutate_erase,
    mutate_inc,
    mutate_dec,
    mutate_not
};


// mutate a set of testcases
testcase_t * mutate(testcase_list_t * testcase_list){
    // select a test case
    testcase_t * testcase = list_testcase_rand(testcase_list);
  
    // decide whether to splice with another testcase here...

    // perform the mutation 
    return mutate_testcase(testcase);
}

// mutate a testcase
testcase_t * mutate_testcase(testcase_t * t){
    int i; 
    lm_debug("Provided testcase buf: %p\n", t->buf);

    testcase_t * mutated_testcase;
    lm_malloc(sizeof(testcase_t), mutated_testcase);
    lm_malloc(t->len, mutated_testcase->buf);
    mutated_testcase->len = t->len;
    memcpy(mutated_testcase->buf, t->buf, t->len);

    if(t->len == 0){
        return NULL;
    }

    lm_debug("New testcase buf: %p\n", mutated_testcase->buf);

    // dict stuff goes here...

    // void (* funcs) = mutators->mutator_funcs;
    int total_changes = rand_between(1, fuzzfactor); // fuzzfactor sets the maximum number of stacked mutations
    for(i = 0; i < total_changes; i++){
        uint64_t f = rand_at_most(sizeof(mutation_functions)/sizeof(*mutation_functions)-1);
        lm_debug("Running mutator %lu\n", f);
        mutation_functions[f](mutated_testcase);
    }

    return mutated_testcase;
}

void mutate_set_fuzzfactor(int ff){
    // pthread lock here
    lm_debug("Setting fuzz factor to %d\n", ff);
    fuzzfactor = ff;
}

/*
* Bit mutations
*/

// flip a random bit
inline void mutate_bit(testcase_t * t){
    uint64_t bit = rand_at_most((t->len<<3)-1);
    FLIP_BIT(t->buf, bit);
}

// flip 2 adjacent bits
void mutate_bit2(testcase_t * t){
    uint64_t bit = rand_at_most((t->len<<3)-2);
    
    FLIP_BIT(t->buf, bit);
    FLIP_BIT(t->buf, bit+1);
}

// flip 4 adjacent bits
void mutate_bit4(testcase_t * t){
    uint64_t bit = rand_at_most((t->len<<3)-4);

    FLIP_BIT(t->buf, bit);
    FLIP_BIT(t->buf, bit+1);
    FLIP_BIT(t->buf, bit+2);
    FLIP_BIT(t->buf, bit+3);
}

/*
* Byte mutations
*/

// set a random byte
void mutate_byte(testcase_t * t){
    uint64_t off = rand_at_most(t->len-1);
    lm_debug("setting %lu\n", off);
    t->buf[off] = (uint8_t)rand_next();
}

// set a randomly sized chunk of buf to random values
void mutate_random(testcase_t * t){
    uint64_t offset = rand_at_most(t->len - 1);
    uint64_t len = rand_between(1, t->len - offset);
    rand_buf(&t->buf[offset], len);
}

// insert a random byte
void mutate_insert_byte(testcase_t * t){
    uint64_t off = rand_at_most(t->len - 1);
    
    lm_debug("inserting at %lu\n", off);
    t->buf = realloc(t->buf, t->len+1);
    if(t->buf == NULL){
        fatal("realloc() failed");
    }

    memmove(t->buf + off + 1, t->buf + off, t->len-off);
    t->buf[off] = (uint8_t)rand_next();
    t->len += 1;
}

// insert up to t->len random bytes
void mutate_insert_bytes(testcase_t * t){
    uint64_t off = rand_at_most(t->len - 1);
    uint64_t len = rand_at_most(t->len);
    
    lm_debug("inserting at %lu len %lu\n", off, len);
    t->buf = realloc(t->buf, t->len+len);
    if(t->buf == NULL){
        fatal("realloc() failed");
    }

    memmove(t->buf+off+len, t->buf + off, t->len-off);
    uint64_t i;
    for(i = 0; i < len; i++){
        t->buf[off+i] = (uint8_t)rand_next();
    }

    t->len += len;
}

// insert up to t->len of printable ASCII bytes
void mutate_insert_ascii_bytes(testcase_t * t){
    uint64_t off = rand_at_most(t->len -1);
    uint64_t len = rand_at_most(t->len);

    lm_debug("inserting ASCII bytes at %lu len %lu\n", off, len);
    t->buf = realloc(t->buf, t->len+len);

    if(t->buf == NULL){
        fatal("realloc() failed");
    }

    memmove(t->buf+off+len, t->buf +off, t->len-off);
    uint64_t i;
    for(i = 0; i < len; i++){
        t->buf[off+i] = (uint8_t)rand_between(32,136);
    }

    t->len += len;
}        


// shuffle a block of bytes, no more than 8 bytes
void mutate_shuffle(testcase_t * t){
    if(t->len <= 1){
        return;
    }
    else if (t->len == 2){
        uint8_t a = t->buf[1];
        t->buf[1] = t->buf[0];
        t->buf[0] = a;
        return;
    }
  
    uint64_t len = (t->len > 8) ? rand_between(2, 8) : rand_between(2, t->len-1);
    uint64_t offset = rand_at_most(t->len - len);

    lm_debug("shuffling len: %lu offset: %lu\n", len, offset);
    
    if(unlikely((len + offset) > t->len)){
        fatal("mutate_shuffle out of bounds");
    }

    uint64_t i;
    for(i = 0; i < len-1; i++){
        uint64_t off = rand_at_most(len-1);
        uint8_t a = t->buf[off+offset];
        t->buf[off+offset] = t->buf[i+offset];
        t->buf[i+offset] = a;
    }
}

// erase a random number of bytes, no more than len/2
void mutate_erase(testcase_t * t){
    uint64_t len = rand_at_most(t->len/2);
    uint64_t offset = rand_at_most(t->len - len);
    
    if(unlikely((len + offset) > t->len)){
        fatal("mutate_erase out of bounds: len: %lu offset: %lu", len, offset);
    }

    lm_debug("Erasing len %lu at offset %lu\n", len, offset);
    memmove(t->buf + offset, t->buf + offset + len, t->len - offset - len);
    t->len -= len;
}

/*
* Arithmetic mutations
*/

// increment a random byte
void mutate_inc(testcase_t * t){
    uint64_t offset = rand_at_most(t->len-1);
    lm_debug("Incrementing byte %lu\n", offset);
    t->buf[offset] += 1;
}

// decrement a random byte
void mutate_dec(testcase_t * t){
    uint64_t offset = rand_at_most(t->len-1);
    lm_debug("Decrementing byte %lu\n", offset);
    t->buf[offset] -= 1;
}

// invert a random byte
void mutate_not(testcase_t * t){
    uint64_t offset = rand_at_most(t->len-1);
    lm_debug("Inverting byte %lu\n", offset);
    t->buf[offset] =~ t->buf[offset];
}
