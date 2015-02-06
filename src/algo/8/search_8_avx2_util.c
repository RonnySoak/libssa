/*
 * search_8.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "search_8.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../searcher.h"
#include "../../util/util.h"
#include "../../util/linked_list.h"
#include "../../db_iterator.h"
#include "../../matrices.h"
#include "../16/search_16.h"

static void free_query( p_s8query query ) {
    if( query ) {
        free( query->q_table_avx );
        query->q_len = 0;

        free( query );
    }
}

static void search_8_avx2_init_query( p_s8info s, int q_count, seq_buffer * queries ) {
    s->q_count = q_count;

    s->maxqlen = 0;

    for( int i = 0; i < q_count; ++i ) {
        if( s->queries[i] )
            free_query( s->queries[i] );

        p_s8query query = (p_s8query) xmalloc( sizeof(struct s8query) );

        query->q_len = queries[i].seq.len;
        query->seq = queries[i].seq.seq;

        if( query->q_len > s->maxqlen ) {
            s->maxqlen = query->q_len;
        }

        query->q_table_avx = (__m256i **) xmalloc( query->q_len * sizeof(__m256i *) );

        for( int j = 0; j < query->q_len; j++ )
            /*
             * q_table holds pointers to dprofile, which holds the actual query data.
             * The dprofile is filled during the search for every four columns, that are searched.
             */
            query->q_table_avx[j] = s->dprofile_avx + CDEPTH_8_BIT * (int) (queries[i].seq.seq[j]);

        s->queries[i] = query;
    }

    if( s->hearray_avx )
        free( s->hearray_avx );
    s->hearray_avx = (__m256i *) xmalloc( 2 * s->maxqlen * sizeof(__m256i ) );
    memset( s->hearray_avx, 0, 2 * s->maxqlen * sizeof(__m256i ) );
}

p_s8info search_8_avx2_init( p_search_data sdp ) {
    /* prepare alloc of qtable, dprofile, hearray, dir */
    p_s8info s = (p_s8info) xmalloc( sizeof(struct s8info) );

    s->dprofile_avx = (__m256i *) xmalloc( sizeof(int8_t) * CDEPTH_8_BIT * CHANNELS_8_BIT * SCORE_MATRIX_DIM );
    s->q_count = 0;
    s->hearray_avx = 0;
    s->maxdlen = ssa_db_get_longest_sequence();

    for( int i = 0; i < 6; i++ ) {
        s->queries[i] = 0;
    }

    s->penalty_gap_open = gapO;
    s->penalty_gap_extension = gapE;

    s->s16info = 0;

    search_8_avx2_init_query( s, sdp->q_count, sdp->queries );

    return s;
}

void search_8_avx2_exit( p_s8info s ) {
    /* free mem for dprofile, hearray, dir, qtable */
    if( s->hearray_avx )
        free( s->hearray_avx );
    if( s->dprofile_avx )
        free( s->dprofile_avx );

    for( int i = 0; i < s->q_count; i++ ) {
        free_query( s->queries[i] );
    }
    s->q_count = 0;

    if( s->s16info ) {
        search_16_sse2_exit( s->s16info );
        s->s16info = 0;
    }

    free( s );
}

void dprofile_fill_8_avx2( int8_t * dprofile, uint8_t * dseq_search_window ) {
    // TODO

    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

    // 4 x 16 db symbols
    // ca (60x2+68x2)x4 = 976 instructions TODO verify these numbers

    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        int d[CHANNELS_8_BIT];

        for( int i = 0; i < CHANNELS_8_BIT; i++ )
            d[i] = dseq_search_window[j * CHANNELS_8_BIT + i] << 5;

        for( int i = 0; i < SCORE_MATRIX_DIM; i += 8 ) {
            xmm0 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[0]) );
            xmm1 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[1]) );
            xmm2 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[2]) );
            xmm3 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[3]) );
            xmm4 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[4]) );
            xmm5 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[5]) );
            xmm6 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[6]) );
            xmm7 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[7]) );
            xmm8 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[8]) );
            xmm9 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[9]) );
            xmm10 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[10]) );
            xmm11 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[11]) );
            xmm12 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[12]) );
            xmm13 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[13]) );
            xmm14 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[14]) );
            xmm15 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + i + d[15]) );

            xmm0 = _mm_unpacklo_epi8( xmm0, xmm1 );
            xmm2 = _mm_unpacklo_epi8( xmm2, xmm3 );
            xmm4 = _mm_unpacklo_epi8( xmm4, xmm5 );
            xmm6 = _mm_unpacklo_epi8( xmm6, xmm7 );
            xmm8 = _mm_unpacklo_epi8( xmm8, xmm9 );
            xmm10 = _mm_unpacklo_epi8( xmm10, xmm11 );
            xmm12 = _mm_unpacklo_epi8( xmm12, xmm13 );
            xmm14 = _mm_unpacklo_epi8( xmm14, xmm15 );

            xmm1 = xmm0;
            xmm0 = _mm_unpacklo_epi16( xmm0, xmm2 );
            xmm1 = _mm_unpackhi_epi16( xmm1, xmm2 );
            xmm5 = xmm4;
            xmm4 = _mm_unpacklo_epi16( xmm4, xmm6 );
            xmm5 = _mm_unpackhi_epi16( xmm5, xmm6 );
            xmm9 = xmm8;
            xmm8 = _mm_unpacklo_epi16( xmm8, xmm10 );
            xmm9 = _mm_unpackhi_epi16( xmm9, xmm10 );
            xmm13 = xmm12;
            xmm12 = _mm_unpacklo_epi16( xmm12, xmm14 );
            xmm13 = _mm_unpackhi_epi16( xmm13, xmm14 );

            xmm2 = xmm0;
            xmm0 = _mm_unpacklo_epi32( xmm0, xmm4 );
            xmm2 = _mm_unpackhi_epi32( xmm2, xmm4 );
            xmm6 = xmm1;
            xmm1 = _mm_unpacklo_epi32( xmm1, xmm5 );
            xmm6 = _mm_unpackhi_epi32( xmm6, xmm5 );
            xmm10 = xmm8;
            xmm8 = _mm_unpacklo_epi32( xmm8, xmm12 );
            xmm10 = _mm_unpackhi_epi32( xmm10, xmm12 );
            xmm14 = xmm9;
            xmm9 = _mm_unpacklo_epi32( xmm9, xmm13 );
            xmm14 = _mm_unpackhi_epi32( xmm14, xmm13 );

            xmm3 = xmm0;
            xmm0 = _mm_unpacklo_epi64( xmm0, xmm8 );
            xmm3 = _mm_unpackhi_epi64( xmm3, xmm8 );
            xmm7 = xmm2;
            xmm2 = _mm_unpacklo_epi64( xmm2, xmm10 );
            xmm7 = _mm_unpackhi_epi64( xmm7, xmm10 );
            xmm11 = xmm1;
            xmm1 = _mm_unpacklo_epi64( xmm1, xmm9 );
            xmm11 = _mm_unpackhi_epi64( xmm11, xmm9 );
            xmm15 = xmm6;
            xmm6 = _mm_unpacklo_epi64( xmm6, xmm14 );
            xmm15 = _mm_unpackhi_epi64( xmm15, xmm14 );

            _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64 * i + 0), xmm0 );
            _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64 * i + 64), xmm3 );
            _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64 * i + 128), xmm2 );
            _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64 * i + 192), xmm7 );
            _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64 * i + 256), xmm1 );
            _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64 * i + 320), xmm11 );
            _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64 * i + 384), xmm6 );
            _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64 * i + 448), xmm15 );
        }
    }
}
