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

/**
 * TODO
 *
 * add functions, to get chunks of the DB
 */

// #############################################################################
// Data types
// ##########

typedef struct seqinfo {
    char * header;
    char * seq;
    unsigned long ID;
    unsigned long headerlen;
    unsigned long seqlen;
} seqinfo;

typedef struct seqinfo* p_seqinfo;

// #############################################################################
// Initialisation
// ##############
void ssa_db_init_fasta(const char* fasta_file_name);

// #############################################################################
// Accessors
// #########
/**
 * Returns the next sequence available in the database.
 *
 * TODO better to copy the data to a new instance, to prevent alteration of the
 *      DB from the outside, or maybe do it on the DB side ...
 */
p_seqinfo ssa_db_next_sequence();

/**
 * Releases the memory allocated by the function sdb_init_fasta.
 *
 * @see sdb_init_fasta
 */
void ssa_db_free();

#endif /* LIBSSA_EXTERN_DB_H_ */
