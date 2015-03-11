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

#ifndef LIBSSA_EXTERN_DB_H_
#define LIBSSA_EXTERN_DB_H_

#include <stddef.h>

// #############################################################################
// Data types
// ##########
typedef struct {
    size_t ID;
    size_t seqlen;
    char * seq;
} seqinfo_t;

typedef seqinfo_t * p_seqinfo;

// #############################################################################
// Initialisation
// ##############
int ssa_db_init( const char * db_name );

// #############################################################################
// Accessors
// #########
size_t ssa_db_get_sequence_count();

/**
 * Returns the DB sequence of the specified ID.
 */
p_seqinfo ssa_db_get_sequence( size_t id );

/**
 * Releases the memory allocated by the function sdb_init_fasta.
 *
 * @see sdb_init_fasta
 */
void ssa_db_close();

#endif /* LIBSSA_EXTERN_DB_H_ */
