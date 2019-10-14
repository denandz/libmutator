#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mutate.h"
#include "random.h"
#include "util.h"

// mutate a packet, expect a different packet back
int main(){
    uint8_t * input;
    lm_malloc(1024, input);
    memset(input, 0x00, 1024);
    uint64_t i = 0;
    uint64_t bufflen = 1024;

    while(1){
        int c = getchar();
        if(c == EOF)
            break;
        input[i] = c;
        i++;
        if(i > bufflen){
            input = realloc(input, bufflen+1024);
            bufflen += 1024;
        }
    }

    lm_debug("Got input len %lu\n", i);

    dump_hex(input, i);

    testcase_t * a = testcase_load(input, i);
    testcase_t * b = mutate_testcase(a);

    printf("len: %lu\n", b->len);
    dump_hex(b->buf, b->len);

    free(input);
    testcase_free(a);
    testcase_free(b);
    return 1;
}
