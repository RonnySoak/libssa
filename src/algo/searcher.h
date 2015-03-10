/*
 * searcher.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCHER_H_
#define SEARCHER_H_

#include "../libssa_datatypes.h"

#define SMITH_WATERMAN 0
#define NEEDLEMAN_WUNSCH 1
#define NEEDLEMAN_WUNSCH_SELLERS 2

void s_init( int search_type, int bit_width, p_query query );

p_search_data s_create_searchdata( p_query query );

void s_free_search_data( p_search_data sdp );

int s_get_query_count();

seq_buffer_t s_get_query( int idx );

void s_free();

void * s_search( void * hit_count );

#endif /* SEARCHER_H_ */
