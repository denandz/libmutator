#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../mutate.h"
#include "../testcase.h"
#include "util.h"

// load all the files in a directory into a testcase list, perform 20 mutations
int main(){
    
    testcase_list_t * t = list_testcase_load_dir("./cases");

    int i;
    for(i = 0; i < 20; i++){
        testcase_t * mutated = mutate(t);
        dump_hex(mutated->buf, mutated->len);
        testcase_free(mutated);
    }

    list_testcase_free(t);

    return 1;
}
