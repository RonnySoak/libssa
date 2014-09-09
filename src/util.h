/*
 * util.h
 *
 *  Created on: 18 Jul 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// used in matrices.c
extern char * matrixname;
extern int symtype;
extern long matchscore;
extern long mismatchscore;

extern char map_ncbi_aa[];

#ifndef LINE_MAX
#define LINE_MAX 2048
#endif

void * xmalloc(size_t size);
void * xrealloc(void *ptr, size_t size);

// output data
void ffatal(const char * format, ...);

/** Initialises the output stream. Default is stdout */
void init_out(const char* filename);
/** Writes to the output stream */
void outf(const char* format, ...);
// output data


#endif /* UTIL_H_ */
