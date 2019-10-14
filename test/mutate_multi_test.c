#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../mutate.h"
#include "../testcase.h"
#include "util.h"

// mutate a packet, expect a different packet back
int main(){
    char packet1[] = "test this packet, what will happen?!";
    char packet2[] = "another test packet";
    char packet3[] = "do lizards like, know that they're lizards?";

    testcase_t * a = testcase_load((uint8_t *)packet1, sizeof(packet1));
    printf("testcase len %lu loaded len %lu\n", sizeof(packet1), a->len);
    testcase_t * b = testcase_load((uint8_t *)packet2, sizeof(packet2));
    printf("testcase len %lu loaded len %lu\n", sizeof(packet2), b->len);
    testcase_t * c = testcase_load((uint8_t *)packet3, sizeof(packet3));
    printf("testcase len %lu loaded len %lu\n", sizeof(packet3), c->len);
      
    // init the testcase list
    testcase_list_t * list = testcase_list_init();
    list_testcase_add(list, a);
    list_testcase_add(list, b);
    list_testcase_add(list, c);

    // set the fuzzfactor
    mutate_set_fuzzfactor(5);

    int i;
    for(i = 0; i < 20; i++){
        testcase_t * mutated = mutate(list);
        dump_hex(mutated->buf, mutated->len);
        testcase_free(mutated);
    }

    list_testcase_free(list);

    return 0;
}
