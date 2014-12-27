/*
 * util_sequence.h
 *
 *  Created on: Oct 12, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef UTIL_SEQUENCE_H_
#define UTIL_SEQUENCE_H_

#include <stddef.h>

#include "libssa_datatypes.h"

extern const char map_ncbi_aa[256];
extern const char map_ncbi_nt16[256];

// TODO provide to the user of the lib
extern const char * gencode_names[23];

extern const char * sym_ncbi_nt16;
extern const char * sym_ncbi_nt16u;
extern const char * sym_ncbi_aa;

/**
 * Initialises the translation tables for query and DB sequences, using the
 * provided genetic codes.
 *
 * @see us_translate_sequence()
 *
 * @param qtableno  the genetic code of the query sequences
 * @param dtableno  the genetic code of the DB sequences
 */
void us_init_translation(int qtableno, int dtableno);

/**
 * Returns a newly created sequence, with the mapped values of the input
 * sequence.
 *
 * @param seq   the sequence, to be mapped
 * @param map   the mapping to use
 * @return      the new mapped sequence
 */
void us_map_sequence( sequence orig, sequence mapped, const char* map );

/**
 * Translates a DNA sequence into a protein sequence, according the strand and
 * frame information. If db_sequence is set, the translation table, initialized
 * with the genetic code, of the DB is used. Otherwise the translation table,
 * initialized with the genetic code, of the query sequence is used.
 *
 * @param db_sequence   1 for translating a DB sequence 0 otherwise
 * @param dna       the DNA sequence to translate
 * @param strand    the strand, that is translated
 * @param frame     the frame, that is read for translation
 * @param prot_seq  the resulting protein sequence
 */
void us_translate_sequence(int db_sequence, sequence dna,
        int strand, int frame, sequence * prot_seq);

/**
 * Computes the reverse complement of the input sequence. If the input sequence
 * is empty, the function returns 0.
 *
 * @param seq   the sequence to reverse complement
 * @param len   the length of the sequence
 * @return      the reverse complement or 0 in case of an empty sequence
 */
void us_revcompl( sequence orig, sequence rc );

#endif /* UTIL_SEQUENCE_H_ */
