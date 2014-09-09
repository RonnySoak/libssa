/*
 * util.c
 *
 *  Created on: 18 Jul 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef DATA_H_
#define DATA_H_

#include <stdarg.h>
#include <mm_malloc.h>

#include "libssa.h"
#include "util.h"

#define DEFAULT_MATRIXNAME "blosum62"
#define DEFAULT_MATCHSCORE 1
#define DEFAULT_MISMATCHSCORE (-3)
#define DEFAULT_SYMTYPE AMINOACID

// used in matrices.c
char * matrixname = DEFAULT_MATRIXNAME;
int symtype = DEFAULT_SYMTYPE;
long matchscore = DEFAULT_MATCHSCORE;
long mismatchscore = DEFAULT_MISMATCHSCORE;

FILE* out_file;

void ffatal(const char * format, ...) {
    if (format) {
        va_list arg;
        fprintf(stderr, format, arg);
        fprintf(stderr, "\n");
    }
    exit(1);
}

void outf(const char* format, ...) {
    va_list arg;
    fprintf(out_file, format, arg);
}

void init_out(const char* filename) {
    if(NULL == filename) {
        out_file = stdout;
        outf("Writing to stdout\n");
    }
    else {
        FILE * f = fopen(filename, "w");
        if (!f) {
            ffatal("Unable to open output file for writing.");
        }
        out_file = f;
    }
}

void * xmalloc(size_t size) {
    const size_t alignment = 16;
    void * t;
    if (posix_memalign(&t, alignment, size) != 0) {
        ffatal("Unable to allocate enough memory.");
    }

    return t;
}

void * xrealloc(void *ptr, size_t size) {
    void * t = realloc(ptr, size);
    if (!t) {
        ffatal("Unable to allocate enough memory.");
    }
    return t;
}
// used in matrices.c

#endif /* DATA_H_ */
