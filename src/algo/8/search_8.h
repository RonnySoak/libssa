/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_8_H_
#define SEARCH_8_H_

#include <stdint.h>
#include <immintrin.h>

#include "../16/search_16.h"
#include "../../libssa_datatypes.h"
#include "../../util/minheap.h"
#include "../../util/linked_list.h"

#define CHANNELS_8_BIT 16
#define CDEPTH_8_BIT 4

struct s8query {
    unsigned long q_len;

    __m128i ** q_table;

    char * seq;
};
typedef struct s8query * p_s8query;

struct s8info {
    __m128i * hearray;
    __m128i * dprofile;

    unsigned long maxdlen;
    unsigned long maxqlen;

    int q_count;
    p_s8query queries[6];

    int8_t penalty_gap_open;
    int8_t penalty_gap_extension;

    p_s16info s16info;
};
typedef struct s8info * p_s8info;

void search8_init_algo( int search_type );

void dprofile_fill8( int8_t * dprofile, uint8_t * dseq );

int move_db_sequence_window_8( int c, uint8_t* d_begin[CHANNELS_8_BIT], uint8_t* d_end[CHANNELS_8_BIT], uint8_t* dseq );

void search_8_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

void search_8_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

void search8_nw_sellers( p_s8info s, p_db_chunk chunk, p_minheap heap, int query_id );

void search_8( p_db_chunk chunk, p_search_data sdp, p_search_result res );

#endif /* SEARCH_8_H_ */
