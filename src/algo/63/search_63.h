/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_63_H_
#define SEARCH_63_H_

#include <stdint.h>

#include "../../libssa_datatypes.h"

void init_algo_63( int search_type );

int64_t full_sw(sequence * dseq, sequence * qseq, int64_t * hearray );

int64_t full_nw(sequence * dseq, sequence * qseq, int64_t * hearray );

int64_t full_nw_sellers(sequence * dseq, sequence * qseq, int64_t * hearray );

void search_63( p_db_chunk chunk, p_search_data sdp, p_search_result res );

#endif /* SEARCH_63_H_ */
