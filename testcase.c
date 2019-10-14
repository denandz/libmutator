#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "random.h"
#include "testcase.h"
#include "util.h"

extern int error;

// load a testcase into a testcase_t structure
testcase_t * testcase_load(uint8_t * buf, uint64_t len){
    testcase_t * t;
    lm_malloc(sizeof(testcase_t), t);
    memset(t, 0x00, sizeof(testcase_t));

    lm_malloc(len, t->buf);
    memcpy(t->buf, buf, len);
    t->len = len; //= t->orig_len;

    lm_debug("loaded testcase with len %lu\n", len);
    return t;
}

// load a file into a testcase_t structure
testcase_t * testcase_load_file(char * path){
    long bufsize;
    uint8_t * filebuf;
    FILE * fp; 

    if((fp = fopen(path, "r"))== NULL){
        fatal("Could not open file %s: %s\n", path, strerror(errno));
    }   

    if (fseek(fp, 0L, SEEK_END) == 0) {
        bufsize = ftell(fp);
        if (bufsize == -1){
            fatal("Error with ftell: %s", strerror(errno));
            return NULL;
        }
        else if(bufsize == 0){ // handle empty file
            fatal("empty file");
            return NULL;
        }

        // Go back to the start of the file.
        if (fseek(fp, 0L, SEEK_SET) != 0){ 
            fatal("Could not fseek: %s\n", strerror(errno));
            return NULL;
        }

        // Read the entire file into memory.
        lm_malloc(bufsize, filebuf);
        if(fread(filebuf, sizeof(char), bufsize, fp) != (unsigned long)bufsize){
            fatal("fread");
        }

        if ( ferror( fp ) != 0 ){
           fatal("fread: %s\n", strerror(errno));
        }
    }   
    else{
        fatal("Could not fseek: %s\n", strerror(errno));
    }   

    fclose(fp);
    testcase_t * r = testcase_load(filebuf, bufsize);
    free(filebuf);
    return r;
}

// free() the testcase
void testcase_free(testcase_t * t){
    free(t->buf);
    t->buf = NULL;
    free(t);
    t = NULL;
}

// Create a testcase list
testcase_list_t * testcase_list_init(){
    testcase_list_t * testcase_list;
    lm_malloc(sizeof(testcase_list_t), testcase_list);
    memset(testcase_list, 0x00, sizeof(testcase_list_t));
    testcase_list->count = 0;

    return testcase_list;
}

// Add testcase to list
void list_testcase_add(testcase_list_t * list, testcase_t * t){
    ++list->count;
    lm_debug("New list count: %u\n", list->count);

    list->testcases = realloc(list->testcases, sizeof(testcase_t)*list->count+1);
    if(list->testcases == NULL){
        fatal("realloc failed");
    }

    list->testcases[list->count-1] = t;
}

// return a random testcase from the list
testcase_t * list_testcase_rand(testcase_list_t * list){
    uint32_t entry = rand_at_most(list->count-1);
    lm_debug("selecting testcase %u. list->count %u\n", entry, list->count);
    return list->testcases[entry];
}

// splice two testcases together
testcase_t * splice_testcase(testcase_t * a, testcase_t * b){
    uint64_t off = rand_at_most(a->len - 1);
    uint8_t * buf;
    testcase_t * t;

    lm_malloc(sizeof(b->len)+off, buf);

    memcpy(buf, a->buf, off);
    memcpy(buf+off, b->buf, b->len);
    t = testcase_load(buf, off+b->len);

    free(buf);
    return t;
}

// load all testcases from a given dir
testcase_list_t * list_testcase_load_dir(char * path){
    testcase_list_t * t = testcase_list_init();
    uint8_t * buf;
    DIR * dir;
    struct dirent *ents;

    if((dir = opendir(path)) == NULL){
        fatal("Could not open directory %s: %s\n", path, strerror(errno));
    }

    while ((ents = readdir(dir)) != NULL){
        // The below assumes a filesystem that supports returning types in dirent structs.
        if(ents->d_type != DT_REG)
            continue;

        FILE * fp;
        char file_path[PATH_MAX];

        snprintf(file_path, PATH_MAX, "%s/%s", path, ents->d_name);
        if((fp = fopen(file_path, "r"))== NULL){
            fatal("Could not open file %s: %s\n", file_path, strerror(errno));
        }

        if (fseek(fp, 0L, SEEK_END) == 0) {
            long bufsize = ftell(fp);
            if (bufsize == -1){
                fatal("Error in ftell: %s", strerror(errno));
            }
            else if(bufsize == 0){ // handle empty file
                fclose(fp);
                continue;
            }

            // Go back to the start of the file.
            if (fseek(fp, 0L, SEEK_SET) != 0){
                fatal("Could not fseek: %s\n", strerror(errno));
            }

            // Read the entire file into memory.
            lm_malloc(bufsize, buf);
            if(fread(buf, sizeof(uint8_t), bufsize, fp) != (size_t)bufsize){
                fatal("fread");
            }

            if ( ferror( fp ) != 0 ){
                fatal("fread: %s\n", strerror(errno));
            }
        
            testcase_t * tcase = testcase_load(buf, bufsize);
            list_testcase_add(t, tcase);
            free(buf);
        }
        
        fclose(fp);
    }

    closedir(dir);

    if(t->count == 0){ // no cases found
        list_testcase_free(t);
        return NULL;
    }

    return t; // place holder
}

void list_testcase_free(testcase_list_t * t){
    uint32_t i;
    for(i = 0; i < t->count; i++){
        testcase_free(t->testcases[i]);
    }

    free(t->testcases);
    free(t);
}
