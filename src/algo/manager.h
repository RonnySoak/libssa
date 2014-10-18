/*
 * manager.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include "../libssa.h"

void free_search_data();

void init_for_sw();

void init_for_nw();

void init_for_nw_sellers();

/**
 * Run a search for query in the database. Aligns the query sequence against
 * each sequence in the DB and returns 'hit_count' alignments. The search is
 * configured through set bits in 'flags'.
 *
 * Flags: TODO
 * Bit 1 set: do Smith-Waterman, otherwise do Needleman-Wunsch
 * Bit 2 set: use SIMD, if possible, otherwise use the naive implementation
 * Bit 3 set: run in multiple threads (configured through the API)
 */
p_alignment_list m_run(p_query query, int hit_count);

#endif /* MANAGER_H_ */
