/*
 Copyright (C) 2014-2015 Jakob Frielingsdorf

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Contact: Jakob Frielingsdorf <jfrielingsdorf@gmail.com>
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>

#include "../libssa.h"
#include "../libssa_datatypes.h"
#include "minheap.h"

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

void add_to_minheap( p_minheap heap, uint8_t query_id, p_sdb_sequence db_seq, long score );

#endif /* UTIL_H_ */
