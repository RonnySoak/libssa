/*
 * manager.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include "../libssa_datatypes.h"

void free_search_data();

void init_for_sw(p_query query, int hit_count);

void init_for_nw(p_query query, int hit_count);

void init_for_nw_sellers(p_query query, int hit_count);

/**
 * Run a search for query in the database. Aligns the query sequence against
 * each sequence in the DB and returns 'hit_count' alignments. The search is
 * configured through set bits in 'flags'.
 */
p_alignment_list m_run();

#endif /* MANAGER_H_ */