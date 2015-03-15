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

region_t init_region_for_global(sequence_t a_seq, sequence_t b_seq);

region_t find_region_and_score_for_local(sequence_t a_seq, sequence_t b_seq);

cigar_p compute_cigar_for_nw(sequence_t a_seq, sequence_t b_seq);

cigar_p compute_cigar_for_sw(sequence_t a_seq, sequence_t b_seq, region_t region);

void free_cigar(cigar_p cigar);

void align_nw(p_alignment alignment);

void align_sw(p_alignment alignment);

#endif /* ALIGN_H_ */
