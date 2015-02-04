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
#include "../../util/linked_list.h"

#define CHANNELS_16_BIT 8
#define CDEPTH_16_BIT 4

struct s16query {
    unsigned long q_len;

    __m128i ** q_table;

    char * seq;
};
typedef struct s16query * p_s16query;

struct s16info {
    __m128i * hearray;
    __m128i * dprofile;

    unsigned long maxdlen;
    unsigned long maxqlen;

    int q_count;
    p_s16query queries[6];

    int16_t penalty_gap_open;
    int16_t penalty_gap_extension;
};
typedef struct s16info * p_s16info;

void search16_init_algo( int search_type );
p_s16info search_16_init( p_search_data sdp );

void search16_exit( p_s16info s );

void dprofile_fill16( int16_t * dprofile, uint8_t * dseq );

int move_db_sequence_window_16( int c, uint8_t* d_begin[CHANNELS_16_BIT], uint8_t* d_end[CHANNELS_16_BIT], uint8_t* dseq_search_window );

void search_16_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

void search_16_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

void search16_nw_sellers( p_s16info s, p_db_chunk chunk, p_minheap heap, int query_id );

unsigned long search_16_chunk( p_s16info s16info, p_minheap heap, p_db_chunk chunk, p_search_data sdp );
void search_16( p_db_chunk chunk, p_search_data sdp, p_search_result res );

#endif /* SEARCH_16_H_ */
