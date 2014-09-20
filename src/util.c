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

#endif /* DATA_H_ */
