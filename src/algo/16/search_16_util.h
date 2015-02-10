/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_16_UTIL_H_
#define SEARCH_16_UTIL_H_

#include "search_16.h"

static inline int move_db_sequence_window_16( int c, int channels, uint8_t ** d_begin, uint8_t ** d_end,
        uint8_t* dseq_search_window ) {
    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        if( d_begin[c] < d_end[c] ) {
            dseq_search_window[channels * j + c] = *(d_begin[c]++);
        }
        else {
            dseq_search_window[channels * j + c] = 0;
        }
    }

    if( d_begin[c] == d_end[c] )
        return 0;
    return 1;
}

static inline void check_min_max( int channels, uint8_t * overflow, int16_t * h_min, int16_t * h_max, int16_t score_min,
        int16_t score_max ) {
    for( int c = 0; c < channels; c++ ) {
        if( (h_min[c] <= score_min) || (h_max[c] >= score_max) ) {
            overflow[c] = 1;
        }
    }
}

static inline void check_max( int channels, uint8_t * overflow, int16_t * h_max, int16_t score_max ) {
    for( int c = 0; c < channels; c++ ) {
        if( h_max[c] >= score_max ) {
            overflow[c] = 1;
        }
    }
}

void search_16_sse2_init( p_search_data sdp, p_s16info s );
void search_16_avx2_init( p_search_data sdp, p_s16info s );

void dprofile_fill_16_sse2( int16_t * dprofile, uint8_t * dseq_search_window );
void dprofile_fill_16_avx2( int16_t * dprofile, uint8_t * dseq_search_window );

void search_16_sse2_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_16_sse2_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_16_sse2_nw_sellers( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

void search_16_avx2_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_16_avx2_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );
void search_16_avx2_nw_sellers( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int query_id );

#endif /* SEARCH_16_UTIL_H_ */
