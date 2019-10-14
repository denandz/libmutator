#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "mutate.h"
#include "random.h"
#include "util.h"
#include <errno.h>

extern int errno;

int main(int argc, char ** argv){
    if(argc < 2)
        fatal("run with ./mutate_file <file>");

    int fd;
    fd = open(argv[1], O_RDWR);
    if(fd == -1){
        fatal("open failed: %s", strerror(errno));
    }
    
    FILE * fp;
    if((fp = fopen(argv[1], "r"))== NULL){
        fatal("[!] Error: Could not open file %s: %s\n", argv[1], strerror(errno));
    }

    uint8_t * file;
    long bufsize;
    
    if (fseek(fp, 0L, SEEK_END) == 0) {
        bufsize = ftell(fp);
        if (bufsize == -1){
            fatal("[!] Error with ftell: %s", strerror(errno));
        }
        else if(bufsize == 0){ // handle empty file
            fatal("empty file");
        }

        // Go back to the start of the file.
        if (fseek(fp, 0L, SEEK_SET) != 0){
            fatal("[!] Error: could not fseek: %s\n", strerror(errno));
        }

        // Read the entire file into memory.
        lm_malloc(bufsize, file);
        if(fread(file, sizeof(char), bufsize, fp) != (unsigned long)bufsize){
            fatal("[!] Error: fread");
        }

        if ( ferror( fp ) != 0 ){
            fatal("[!] Error: fread: %s\n", strerror(errno));
        }
    }
    else{
        fatal("fseek");
    }
    fclose(fp);

#ifdef DEBUG
    dump_hex(file, bufsize);
#endif

    testcase_t * a = testcase_load(file, bufsize);
    testcase_t * b = mutate_testcase(a);

#ifdef DEBUG
    dump_hex(b->buf, b->len);
#endif

    fwrite(a->buf, a->len, sizeof(uint8_t), stdout);

    testcase_free(a);
    testcase_free(b);
    free(file);
    return 1;
}
