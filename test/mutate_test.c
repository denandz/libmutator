#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "mutate.h"
#include "util.h"

// mutate a packet, expect a different packet back
int main(){
    char packet[] = "test this packet, what will happen?!";
    testcase_t * a = testcase_load((uint8_t *)packet, sizeof(packet));
    testcase_t * b;

    printf("testcase len %lu loaded len %lu\n", sizeof(packet), a->len);

    int i;
    for(i = 0; i < 8; i++){
        b = mutate_testcase(a);
        dump_hex(b->buf, b->len);
        testcase_free(b);
    }

    testcase_free(a);
    return 1;
}
