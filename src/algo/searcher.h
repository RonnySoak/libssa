/*
 * searcher.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCHER_H_
#define SEARCHER_H_

#include "../libssa_datatypes.h"
#include "search.h"

#define SMITH_WATERMAN 0
#define NEEDLEMAN_WUNSCH 1
#define NEEDLEMAN_WUNSCH_SELLERS 2

void s_init( int search_type, int bit_width, p_query query, int hit_count );

int s_get_query_count();

seq_buffer s_get_query( int idx );

void s_free();

void * s_search(void *);

#endif /* SEARCHER_H_ */
