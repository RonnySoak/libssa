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

#ifndef SEARCH_16_UTIL_H_
#define SEARCH_16_UTIL_H_

#include "search_16.h"

#include "../../util/util.h"

#define CDEPTH_16_BIT 4
#define CHANNELS_16_BIT_SSE (128 / 16)
#define CHANNELS_16_BIT_AVX (256 / 16)

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

    size_t maxqlen;

    uint16_t penalty_gap_open;
    uint16_t penalty_gap_extension;

    uint8_t q_count;
    p_s16query queries[6];

    int64_t * hearray_64;
};

static inline uint8_t move_db_sequence_window_16( uint8_t c, uint8_t * d_begin[CHANNELS_16_BIT],
        uint8_t * d_end[CHANNELS_16_BIT], uint8_t dseq_search_window[CHANNELS_16_BIT * CDEPTH_16_BIT] ) {
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

void dprofile_fill_16_sse2( __mxxxi * dprofile, uint8_t * dseq_search_window );
void dprofile_fill_16_avx2( __mxxxi * dprofile, uint8_t * dseq_search_window );

void search_16_sse2_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t query_id );
void search_16_sse2_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t query_id );

void search_16_avx2_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t query_id );
void search_16_avx2_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t query_id );

#endif /* SEARCH_16_UTIL_H_ */
