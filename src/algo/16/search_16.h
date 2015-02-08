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

#define CDEPTH_16_BIT 4
#define CHANNELS_16_BIT_SSE (128 / 16)
#define CHANNELS_16_BIT_AVX (256 / 16)

struct s16query {
    unsigned long q_len;

    __m128i ** q_table_sse;
    __m256i ** q_table_avx;

    char * seq;
};
typedef struct s16query * p_s16query;

struct s16info {
    __m128i * hearray_sse;
    __m128i * dprofile_sse;

    __m256i * hearray_avx;
    __m256i * dprofile_avx;

    int64_t * hearray_64;

    unsigned long maxdlen;
    unsigned long maxqlen;

    uint8_t q_count;
    p_s16query queries[6];

    int16_t penalty_gap_open;
    int16_t penalty_gap_extension;
};
typedef struct s16info * p_s16info;

void search_16_init_algo( int search_type );
p_s16info search_16_init( p_search_data sdp );

void search_16_exit( p_s16info s );

unsigned long search_16_chunk( p_s16info s16info, p_minheap heap, p_db_chunk chunk, p_search_data sdp );
void search_16( p_db_chunk chunk, p_search_data sdp, p_search_result res );

#endif /* SEARCH_16_H_ */
