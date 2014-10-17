/*
 * align.h
 *
 *  Created on: Oct 16, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef ALIGN_H_
#define ALIGN_H_

#include <stdint.h>

#include "../libssa_datatypes.h"

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

typedef struct {
    uint64_t a_begin;
    uint64_t a_end;
    uint64_t b_begin;
    uint64_t b_end;
} region_t;

typedef struct {
    char * cigar;
    uint64_t len;
    uint64_t allocated_size;
} cigar_t;

typedef cigar_t * cigar_p;

void init_align_function(void (* align_function) (alignment_p));

region_t init_region_for_global(sequence a_seq, sequence b_seq);

region_t find_region_and_score_for_local(sequence a_seq, sequence b_seq);

cigar_p compute_cigar_for_nw(sequence a_seq, sequence b_seq);

cigar_p compute_cigar_for_sw(sequence a_seq, sequence b_seq, region_t region);

void free_cigar(cigar_p cigar);

void align_nw_sellers(alignment_p alignment);

void align_nw(alignment_p alignment);

void align_sw(alignment_p alignment);

#endif /* ALIGN_H_ */
