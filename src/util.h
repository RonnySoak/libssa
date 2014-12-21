/*
 * util.h
 *
 *  Created on: 18 Jul 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>

#include "libssa.h"

#define CMP_ASC(a,b) (a > b ? -1 : (a < b) ? +1 : 0) // TODO is this really ascending
//#define CMP_DESC(a,b) (a > b ? +1 : (a < b) ? -1 : 0)

// used in matrices.c
#define DEFAULT_MATRIXNAME "blosum62"
#define DEFAULT_MATCHSCORE 1
#define DEFAULT_MISMATCHSCORE (-3)

#define DEFAULT_SYMTYPE AMINOACID
#define DEFAULT_STRAND FORWARD_STRAND

#define DEFAULT_QUERY_GENCODE 1
#define DEFAULT_DB_GENCODE 1

#ifndef LINE_MAX
#define LINE_MAX 2048
#endif

void * xmalloc(size_t size);
void * xrealloc(void *ptr, size_t size);

// output data
void ffatal(const char * format, ...);

/** Initialises the output stream. Default is stdout */
void init_out(const char* filename);
/** Closes the file pointer to the output stream, if it is not stdout */
void close_out();
/** Writes to the output stream */
void outf(const char* format, ...);
// output data

char *strdup(const char *str);

#endif /* UTIL_H_ */
