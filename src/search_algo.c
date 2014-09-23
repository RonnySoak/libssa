/*
 * search_algo.c
 *
 *  Created on: Sep 21, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "util.h"

extern long fullsw(char * dseq,
        char * dend,
        char * qseq,
        char * qend,
        long * hearray,
        long * score_matrix,
        uint8_t gapopenextend,
        uint8_t gapextend);

uint8_t gapO;
uint8_t gapE;

/**
 * Searches for a sequence in the database.
 *
 * TODO implement following:
 *  - search for X sequence with highest count first
 *  - do the alignment afterwards
 *  - return best X sequences/alignments
 *  - - X can be defined by the caller
 *
 *  - select right search tool (7, 16, 16s, 63) (based on initialisation)
 *  - selection of DB sequences (use some masking)
 *  -
 */
p_alignment search(p_query query) {

//    long score = fullsw()

    return NULL;
}
