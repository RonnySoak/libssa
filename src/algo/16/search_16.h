/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_16_H_
#define SEARCH_16_H_

#include <stdint.h>
#include <immintrin.h>

#include "../../libssa_datatypes.h"
#include "../../util/minheap.h"

struct s16info {
    __m128i matrix[32];
    __m128i * hearray;
    __m128i * dprofile;
    __m128i ** qtable;

    int qlen;
    int maxqlen;
    int maxdlen;

    int16_t penalty_gap_open;
    int16_t penalty_gap_extension;
};

typedef struct s16info * p_s16info;

void init_algo_16( int search_type );

p_s16info search16_init( int16_t penalty_gap_open, int16_t penalty_gap_extension );

void search16_exit( p_s16info s );

void search16_init_query( p_s16info s, char * qseq, int qlen );

void search16_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, int query_id );

void search16_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, int query_id );

void search16_nw_sellers( p_s16info s, p_db_chunk chunk, p_minheap heap, int query_id );

void search_16( p_db_chunk chunk, p_search_data sdp, p_search_result res );

#endif /* SEARCH_16_H_ */
