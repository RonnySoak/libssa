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
    size_t a_begin;
    size_t a_end;
    size_t b_begin;
    size_t b_end;
} region_t;

typedef struct {
    char * cigar;
    size_t len;
    size_t allocated_size;
} cigar_t;

typedef cigar_t * cigar_p;

void a_init_align_function(void (* align_function) (p_alignment));

region_t init_region_for_global(sequence_t a_seq, sequence_t b_seq);

region_t find_region_and_score_for_local(sequence_t a_seq, sequence_t b_seq);

cigar_p compute_cigar_for_nw(sequence_t a_seq, sequence_t b_seq);

cigar_p compute_cigar_for_sw(sequence_t a_seq, sequence_t b_seq, region_t region);

void free_cigar(cigar_p cigar);

void align_nw_sellers(p_alignment alignment);

void align_nw(p_alignment alignment);

void align_sw(p_alignment alignment);

#endif /* ALIGN_H_ */
