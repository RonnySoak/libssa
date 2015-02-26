/*
 * util.h
 *
 *  Created on: 18 Jul 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>

#include <stdio.h> // TODO remove in production

#include "../libssa.h"
#include "../libssa_datatypes.h"
#include "minheap.h"
#include "linked_list.h"

#include "debug_tools.h" // TODO remove or let it stay?!

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

extern int output_mode;

void * xmalloc( size_t size );
void * xrealloc( void *ptr, size_t size );

// output data
void ffatal( const char * format, ... );

/** Writes to the output stream */
void outf( const char* format, ... );
// output data

char *strdup( const char *str );

void add_to_minheap( p_minheap heap, int query_id, p_sdb_sequence db_seq, long score );

p_db_chunk convert_to_chunk( p_node linked_list );

#endif /* UTIL_H_ */
