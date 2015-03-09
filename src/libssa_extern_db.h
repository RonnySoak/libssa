/*
 * libssa_extern_db.h
 *
 * Adaptor for sequence database implementations used by libssa.
 *
 *  Created on: Sep 26, 2014
 *      Author: Jakob Frielingsdorf
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
unsigned long ssa_db_get_sequence_count();

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
