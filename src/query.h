/*
 * query.h
 *
 *  Created on: Oct 12, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef QUERY_H_
#define QUERY_H_

#include "util/util.h"
#include "libssa_datatypes.h"

/* Describes, if the DB sequences should be translated prior to the alignments.
 * One of: 0 - 4
 * @see sdb_init_symbol_translation in libsdb.h */
extern int symtype;
/* Describes which strands are used in the alignments
 * One of: 1 - 3
 * @see sdb_init_symbol_translation in libsdb.h */
extern int query_strands;

/**
 * Releases the memory used by the query.
 *
 * @param query     pointer to the query memory
 */
void query_free(p_query query);

p_query query_read_from_string( const char * header, const char * sequence );

/**
 * Reads in a query from a file and returns a pointer to a struct holding the
 * query data.
 *
 * @param queryname     name of the file containing the query
 * @return              a pointer to the read query
 */
p_query query_read_from_file( const char * filename );

/**
 * Prints the query to the configured output file.
 *
 * @param query     the query data to print
 *
 * TODO not tested
 */
void query_show(p_query query);

#endif /* QUERY_H_ */
