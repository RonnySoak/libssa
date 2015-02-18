/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_16_UTIL_H_
#define SEARCH_16_UTIL_H_

#include "search_16.h"

#include "../../util/linked_list.h"

#ifdef __AVX2__
typedef __m256i __mxxxi;
#define CHANNELS_16_BIT CHANNELS_16_BIT_AVX
#else
typedef __m128i  __mxxxi;
#define CHANNELS_16_BIT CHANNELS_16_BIT_SSE
#endif

struct s16query {
    size_t q_len;

    __mxxxi ** q_table;

    char * seq;
};
struct s16info {
    __mxxxi * hearray;
    __mxxxi * dprofile;

    int64_t * hearray_64;

    size_t maxqlen;

    uint8_t q_count;
    p_s16query queries[6];

    int16_t penalty_gap_open;
    int16_t penalty_gap_extension;
};

static inline int move_db_sequence_window_16( int c, uint8_t ** d_begin, uint8_t ** d_end, uint8_t* dseq_search_window ) {
    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        if( d_begin[c] < d_end[c] ) {
            dseq_search_window[CHANNELS_16_BIT * j + c] = *(d_begin[c]++);
        }
        else {
            dseq_search_window[CHANNELS_16_BIT * j + c] = 0;
        }
    }

    if( d_begin[c] == d_end[c] )
        return 0;
    return 1;
}

p_s16info search_16_sse2_init( p_search_data sdp );
p_s16info search_16_avx2_init( p_search_data sdp );

void dprofile_fill_16_sse2( int16_t * dprofile, uint8_t * dseq_search_window );
void dprofile_fill_16_avx2( int16_t * dprofile, uint8_t * dseq_search_window );

void search_16_sse2_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_16_sse2_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_16_sse2_nw_sellers( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

void search_16_avx2_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_16_avx2_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_16_avx2_nw_sellers( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

#endif /* SEARCH_16_UTIL_H_ */
