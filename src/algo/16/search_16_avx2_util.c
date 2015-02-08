/*
 * search_16.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "search_16.h"
#include "search_16_util.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../searcher.h"
#include "../63/search_63.h"
#include "../../util/minheap.h"
#include "../../util/linked_list.h"
#include "../../util/util.h"
#include "../../db_iterator.h"
#include "../../cpu_config.h"
#include "../../matrices.h"

static void search_16_avx2_init_query( p_s16info s, int q_count, seq_buffer * queries ) {
    s->q_count = q_count;

    s->maxqlen = 0;

    for( int i = 0; i < q_count; ++i ) {
        p_s16query query = (p_s16query) xmalloc( sizeof(struct s16query) );

        query->q_len = queries[i].seq.len;
        query->seq = queries[i].seq.seq;

        if( query->q_len > s->maxqlen ) {
            s->maxqlen = query->q_len;
        }

        query->q_table_sse = 0;
        query->q_table_avx = (__m256i **) xmalloc( query->q_len * sizeof(__m256i *) );

        for( int j = 0; j < query->q_len; j++ )
            /*
             * q_table holds pointers to dprofile, which holds the actual query data.
             * The dprofile is filled during the search for every four columns, that are searched.
             */

            query->q_table_avx[j] = &s->dprofile_avx[ CDEPTH_16_BIT * (int) (queries[i].seq.seq[j])];

        s->queries[i] = query;
    }

    if( s->hearray_avx )
        free( s->hearray_avx );
    s->hearray_avx = (__m256i *) xmalloc( 2 * s->maxqlen * sizeof(__m256i ) );
    memset( s->hearray_avx, 0, 2 * s->maxqlen * sizeof(__m256i ) );
}

void search_16_avx2_init( p_search_data sdp, p_s16info s ) {
    s->dprofile_avx = (__m256i *) xmalloc( sizeof(int16_t) * CDEPTH_16_BIT * CHANNELS_16_BIT_AVX * SCORE_MATRIX_DIM );

    search_16_avx2_init_query( s, sdp->q_count, sdp->queries );
}

/*
 * dseq_search_window: CDEPTH_16_BIT x 256 bit
 *  - contains CDEPTH_16_BIT symbols of each DB sequence in all channels
 *
 * dprofile: sizeof(int16_t) * CDEPTH_16_BIT * CHANNELS_16_BIT * SCORE_MATRIX_DIM
 *  - contains the values accessed by the pointers in s16info->qtable
 *  - for each symbol of the CDEPTH_16_BIT symbols of each DB sequence, it contains the
 *      corresponding score matrix line
 */
void dprofile_fill_16_avx2( int16_t * dprofile, uint8_t * dseq_search_window ) {
//    __m256i ymm0, ymm1, ymm2, ymm3, ymm4, ymm5, ymm6, ymm7;
//    __m256i ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15;
//
//    // 4 x 16 db symbols
//    // ca (60x2+68x2)x4 = 976 instructions TODO verify these numbers
//
//    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
//        int d[CHANNELS_16_BIT_AVX];
//
//        for( int i = 0; i < CHANNELS_16_BIT_AVX; i++ )
//            d[i] = dseq_search_window[j * CHANNELS_16_BIT_AVX + i] << 5;
//
//        for( int i = 0; i < SCORE_MATRIX_DIM; i += 8 ) {
//            ymm0 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[0]) );
//            ymm1 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[1]) );
//            ymm2 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[2]) );
//            ymm3 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[3]) );
//            ymm4 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[4]) );
//            ymm5 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[5]) );
//            ymm6 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[6]) );
//            ymm7 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[7]) );
//            ymm8 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[8]) );
//            ymm9 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[9]) );
//            ymm10 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[10]) );
//            ymm11 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[11]) );
//            ymm12 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[12]) );
//            ymm13 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[13]) );
//            ymm14 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[14]) );
//            ymm15 = _mm_loadl_epi64( (__m256i *) (score_matrix_7 + i + d[15]) );
//
//            ymm0 = _mm_unpacklo_epi8( ymm0, ymm1 );
//            ymm2 = _mm_unpacklo_epi8( ymm2, ymm3 );
//            ymm4 = _mm_unpacklo_epi8( ymm4, ymm5 );
//            ymm6 = _mm_unpacklo_epi8( ymm6, ymm7 );
//            ymm8 = _mm_unpacklo_epi8( ymm8, ymm9 );
//            ymm10 = _mm_unpacklo_epi8( ymm10, ymm11 );
//            ymm12 = _mm_unpacklo_epi8( ymm12, ymm13 );
//            ymm14 = _mm_unpacklo_epi8( ymm14, ymm15 );
//
//            ymm1 = ymm0;
//            ymm0 = _mm_unpacklo_epi16( ymm0, ymm2 );
//            ymm1 = _mm_unpackhi_epi16( ymm1, ymm2 );
//            ymm5 = ymm4;
//            ymm4 = _mm_unpacklo_epi16( ymm4, ymm6 );
//            ymm5 = _mm_unpackhi_epi16( ymm5, ymm6 );
//            ymm9 = ymm8;
//            ymm8 = _mm_unpacklo_epi16( ymm8, ymm10 );
//            ymm9 = _mm_unpackhi_epi16( ymm9, ymm10 );
//            ymm13 = ymm12;
//            ymm12 = _mm_unpacklo_epi16( ymm12, ymm14 );
//            ymm13 = _mm_unpackhi_epi16( ymm13, ymm14 );
//
//            ymm2 = ymm0;
//            ymm0 = _mm_unpacklo_epi32( ymm0, ymm4 );
//            ymm2 = _mm_unpackhi_epi32( ymm2, ymm4 );
//            ymm6 = ymm1;
//            ymm1 = _mm_unpacklo_epi32( ymm1, ymm5 );
//            ymm6 = _mm_unpackhi_epi32( ymm6, ymm5 );
//            ymm10 = ymm8;
//            ymm8 = _mm_unpacklo_epi32( ymm8, ymm12 );
//            ymm10 = _mm_unpackhi_epi32( ymm10, ymm12 );
//            ymm14 = ymm9;
//            ymm9 = _mm_unpacklo_epi32( ymm9, ymm13 );
//            ymm14 = _mm_unpackhi_epi32( ymm14, ymm13 );
//
//            ymm3 = ymm0;
//            ymm0 = _mm_unpacklo_epi64( ymm0, ymm8 );
//            ymm3 = _mm_unpackhi_epi64( ymm3, ymm8 );
//            ymm7 = ymm2;
//            ymm2 = _mm_unpacklo_epi64( ymm2, ymm10 );
//            ymm7 = _mm_unpackhi_epi64( ymm7, ymm10 );
//            ymm11 = ymm1;
//            ymm1 = _mm_unpacklo_epi64( ymm1, ymm9 );
//            ymm11 = _mm_unpackhi_epi64( ymm11, ymm9 );
//            ymm15 = ymm6;
//            ymm6 = _mm_unpacklo_epi64( ymm6, ymm14 );
//            ymm15 = _mm_unpackhi_epi64( ymm15, ymm14 );
//
//            _mm_store_si128( (__m256i *) (dprofile + 16 * j + 64 * i + 0), ymm0 );
//            _mm_store_si128( (__m256i *) (dprofile + 16 * j + 64 * i + 64), ymm3 );
//            _mm_store_si128( (__m256i *) (dprofile + 16 * j + 64 * i + 128), ymm2 );
//            _mm_store_si128( (__m256i *) (dprofile + 16 * j + 64 * i + 192), ymm7 );
//            _mm_store_si128( (__m256i *) (dprofile + 16 * j + 64 * i + 256), ymm1 );
//            _mm_store_si128( (__m256i *) (dprofile + 16 * j + 64 * i + 320), ymm11 );
//            _mm_store_si128( (__m256i *) (dprofile + 16 * j + 64 * i + 384), ymm6 );
//            _mm_store_si128( (__m256i *) (dprofile + 16 * j + 64 * i + 448), ymm15 );
//        }
//    }
}
