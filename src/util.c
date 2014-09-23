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

#include "util.h"

// used in matrices.c
char * matrixname = DEFAULT_MATRIXNAME;
/* Describes, if the DB sequences should be translated prior to the alignments.
 * One of: 0 - 4
 * @see sdb_init_symbol_translation in libsdb.h */
int symtype = DEFAULT_SYMTYPE;
/* Describes which strands are used in the alignments
 * One of: 1 - 3
 * @see sdb_init_symbol_translation in libsdb.h */
int query_strands = DEFAULT_STRAND;

int query_gencode = DEFAULT_QUERY_GENCODE;
int db_gencode = DEFAULT_DB_GENCODE;

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

void close_out() {
    if (out_file && (out_file != stdout)) {
        if(fclose(out_file)) {
            ffatal("Could not close output file.");
        }
    }
}

void outf(const char* format, ...) {
    if (!out_file) { // TODO test it, might be broken
        init_out(NULL);
    }

    va_list arg;
    fprintf(out_file, format, arg);
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

#endif /* DATA_H_ */
