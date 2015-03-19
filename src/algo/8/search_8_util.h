/*
 Copyright (C) 2014-2015 Jakob Frielingsdorf

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Contact: Jakob Frielingsdorf <jfrielingsdorf@gmail.com>
 */

#ifndef SEARCH_8_UTIL_H_
#define SEARCH_8_UTIL_H_

#include "search_8.h"

#define CDEPTH_8_BIT 4
#define CHANNELS_8_BIT_SSE (128 / 8)
#define CHANNELS_8_BIT_AVX (256 / 8)

#ifdef __AVX2__
typedef __m256i __mxxxi;
#define CHANNELS_8_BIT CHANNELS_8_BIT_AVX
#else // SSE4.1
typedef __m128i  __mxxxi;
#define CHANNELS_8_BIT CHANNELS_8_BIT_SSE
#endif

struct s8query {
    size_t q_len;

    __mxxxi ** q_table;

    char * seq;
};
typedef struct s8query * p_s8query;

struct s8info {
    __mxxxi * hearray;
    __mxxxi * dprofile;

    size_t maxqlen;

    uint8_t penalty_gap_open;
    uint8_t penalty_gap_extension;

    uint8_t q_count;
    p_s8query queries[6];

    p_s16info s16info;
};

static inline uint8_t move_db_sequence_window_8( uint8_t c, uint8_t * d_begin[CHANNELS_8_BIT],
        uint8_t * d_end[CHANNELS_8_BIT], uint8_t dseq_search_window[CHANNELS_8_BIT * CDEPTH_8_BIT] ) {
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        if( d_begin[c] < d_end[c] ) {
            dseq_search_window[CHANNELS_8_BIT * j + c] = *(d_begin[c]++);
        }
        else {
            dseq_search_window[CHANNELS_8_BIT * j + c] = 0;
        }
    }

    if( d_begin[c] == d_end[c] )
        return 0;
    return 1;
}

p_s8info search_8_sse41_init( p_search_data sdp );
p_s8info search_8_avx2_init( p_search_data sdp );

void dprofile_fill_8_sse41( __mxxxi * dprofile, uint8_t * dseq_search_window );
void dprofile_fill_8_avx2( __mxxxi * dprofile, uint8_t * dseq_search_window );

void search_8_sse41_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t query_id );
void search_8_sse41_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t query_id );

void search_8_avx2_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t query_id );
void search_8_avx2_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t query_id );

#endif /* SEARCH_8_UTIL_H_ */
