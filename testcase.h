#include <stdint.h>

#ifndef TESTCASE_H
#define TESTCASE_H

typedef struct {
    uint8_t * buf; // testcase buffer
    uint64_t len; // length of the testcase
    uint64_t num; // number of mutations performed
} testcase_t;

typedef struct {
    uint32_t count;
    testcase_t ** testcases;   
} testcase_list_t; 

void testcase_reset(testcase_t * t);
testcase_t * testcase_load(uint8_t * buf, uint64_t len);
testcase_t * testcase_load_file(char * path);
void testcase_free(testcase_t * t);
testcase_list_t * testcase_list_init();
void list_testcase_add(testcase_list_t * list, testcase_t * t);
testcase_t * list_testcase_rand(testcase_list_t * list);
testcase_list_t * list_testcase_load_dir(char * path);
void list_testcase_free(testcase_list_t * t);

#endif
