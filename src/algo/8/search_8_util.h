/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_8_UTIL_H_
#define SEARCH_8_UTIL_H_

#include "search_8.h"

static inline int move_db_sequence_window_8( int c, uint8_t* d_begin[CHANNELS_8_BIT], uint8_t* d_end[CHANNELS_8_BIT],
        uint8_t dseq_search_window[CDEPTH_8_BIT * CHANNELS_8_BIT] ) {
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        if( d_begin[c] < d_end[c] ) {
            dseq_search_window[CHANNELS_8_BIT * j + c] = *(d_begin[c]++);
        }
        else {
            dseq_search_window[CHANNELS_8_BIT * j + c] = 0;
        }
    }

    if( d_begin[c] == d_end[c] ) // TODO vectorize this check
        return 0;
    return 1;
}

void search_8_sse41_init( p_search_data sdp, p_s8info s );
void search_8_avx2_init( p_search_data sdp, p_s8info s );

void dprofile_fill_8_sse41( int8_t * dprofile, uint8_t * dseq_search_window );
void dprofile_fill_8_avx( int8_t * dprofile, uint8_t * dseq_search_window );

void search_8_sse41_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_8_sse41_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_8_sse41_nw_sellers( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

void search_8_avx2_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_8_avx2_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_8_avx2_nw_sellers( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

#endif /* SEARCH_8_UTIL_H_ */
