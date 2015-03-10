/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_64_H_
#define SEARCH_64_H_

#include <stdint.h>

#include "../../libssa_datatypes.h"

void search_64_init_algo( int search_type );

int64_t* search_64_alloc_hearray( p_search_data sdp );

int64_t full_sw(sequence_t * dseq, sequence_t * qseq, int64_t * hearray );

int64_t full_nw(sequence_t * dseq, sequence_t * qseq, int64_t * hearray );

int64_t full_nw_sellers(sequence_t * dseq, sequence_t * qseq, int64_t * hearray );

void search_64_chunk( p_minheap heap, p_db_chunk chunk, p_search_data sdp, int64_t* hearray );

void search_64( p_db_chunk chunk, p_search_data sdp, p_search_result res );

#endif /* SEARCH_64_H_ */
