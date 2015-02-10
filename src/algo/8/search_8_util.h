/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_8_UTIL_H_
#define SEARCH_8_UTIL_H_

#include "search_8.h"

#include "../../util/linked_list.h"

static inline int move_db_sequence_window_8( int c, int channels, uint8_t ** d_begin, uint8_t ** d_end,
        uint8_t * dseq_search_window ) {
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        if( d_begin[c] < d_end[c] ) {
            dseq_search_window[channels * j + c] = *(d_begin[c]++);
        }
        else {
            dseq_search_window[channels * j + c] = 0;
        }
    }

    if( d_begin[c] == d_end[c] ) // TODO vectorize this check
        return 0;
    return 1;
}

static inline void check_min_max( int channels, uint8_t * overflow, int8_t * h_min, int8_t * h_max, int8_t score_min,
        int8_t score_max ) {
    for( int c = 0; c < channels; c++ ) {
        if( (h_min[c] <= score_min) || (h_max[c] >= score_max) ) {
            overflow[c] = 1;
        }
    }
}

static inline void check_max( int channels, uint8_t * overflow, int8_t * h_max, int8_t score_max ) {
    for( int c = 0; c < channels; c++ ) {
        if( h_max[c] >= score_max ) {
            overflow[c] = 1;
        }
    }
}

void search_8_sse41_init( p_search_data sdp, p_s8info s );
void search_8_avx2_init( p_search_data sdp, p_s8info s );

void dprofile_fill_8_sse41( int8_t * dprofile, uint8_t * dseq_search_window );
void dprofile_fill_8_avx2( int8_t * dprofile, uint8_t * dseq_search_window );

void search_8_sse41_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_8_sse41_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_8_sse41_nw_sellers( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

void search_8_avx2_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_8_avx2_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_8_avx2_nw_sellers( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

#endif /* SEARCH_8_UTIL_H_ */
