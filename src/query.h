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

p_query query_read_from_string( const char * sequence );

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
