/*
 * search_8.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "search_8.h"
#include "search_8_util.h"

#include <stdlib.h>
#include <string.h>

#include "../../util/util.h"
#include "../../matrices.h"
#include "../search.h"

#ifdef __AVX2__

#define CHANNELS_8_BIT CHANNELS_8_BIT_AVX
typedef __m256i __mxxxi;

#else // SSE2

#define CHANNELS_8_BIT CHANNELS_8_BIT_SSE
typedef __m128i  __mxxxi;

#endif

static void search_8_init_query( p_s8info s, int q_count, seq_buffer * queries ) {
    s->q_count = q_count;

    s->maxqlen = 0;

    for( int i = 0; i < q_count; ++i ) {
        p_s8query query = (p_s8query) xmalloc( sizeof(struct s8query) );

        query->q_len = queries[i].seq.len;
        query->seq = queries[i].seq.seq;

        if( query->q_len > s->maxqlen ) {
            s->maxqlen = query->q_len;
        }

        query->q_table = (__mxxxi **) xmalloc( query->q_len * sizeof(__mxxxi *) );

        for( size_t j = 0; j < query->q_len; j++ )
            /*
             * q_table holds pointers to dprofile, which holds the actual query data.
             * The dprofile is filled during the search for every four columns, that are searched.
             */
            query->q_table[j] = &s->dprofile[CDEPTH_8_BIT * (int) (queries[i].seq.seq[j])];

        s->queries[i] = query;
    }

    s->hearray = (__mxxxi *) xmalloc( 2 * s->maxqlen * sizeof(__mxxxi ) );
    memset( s->hearray, 0, 2 * s->maxqlen * sizeof(__mxxxi ) );
}

#ifdef __AVX2__
p_s8info search_8_avx2_init( p_search_data sdp ) {
#else
p_s8info search_8_sse41_init( p_search_data sdp ) {
#endif
    p_s8info s = (p_s8info) xmalloc( sizeof(struct s8info) );

    s->q_count = 0;
    for( int i = 0; i < 6; i++ ) {
        s->queries[i] = 0;
    }

    s->penalty_gap_open = gapO;
    s->penalty_gap_extension = gapE;

    s->s16info = 0;

    s->dprofile = (__mxxxi *) xmalloc( sizeof(int8_t) * CDEPTH_8_BIT * CHANNELS_8_BIT * SCORE_MATRIX_DIM );

    search_8_init_query( s, sdp->q_count, sdp->queries );

    return s;
}

#ifdef __AVX2__
void dprofile_fill_8_avx2( int8_t * dprofile, uint8_t * dseq_search_window ) {
    __m256i ymm[CHANNELS_8_BIT];
    __m256i ymm_t[CHANNELS_8_BIT];

    // 4 x 16 db symbols
    // ca (60x2+68x2)x4 = 976 instructions TODO verify these numbers

#if 0
     dbg_dumpscorematrix_8( score_matrix_7 );

     outf( "DB search window:\n");
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_8_BIT; z++ )
        fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_8_BIT + z]] );
        fprintf( stderr, "\n" );
    }
#endif
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        int d[CHANNELS_8_BIT];

        for( int i = 0; i < CHANNELS_8_BIT; i++ ) // TODO try out to merge this and the next for loop. Does it save cycles?
            d[i] = dseq_search_window[j * CHANNELS_8_BIT + i] << 5;

        // load matrix
        for( int i = 0; i < CHANNELS_8_BIT; i++ ) {
            ymm[i] = _mm256_load_si256( (__m256i *) (score_matrix_7 + d[i]) );
        }

        // transpose matrix
        for( int i = 0; i < CHANNELS_8_BIT; i += 2 ) {
            ymm_t[i + 0] = _mm256_unpacklo_epi8( ymm[i + 0], ymm[i + 1] );
            ymm_t[i + 1] = _mm256_unpackhi_epi8( ymm[i + 0], ymm[i + 1] );
        }

        for( int i = 0; i < CHANNELS_8_BIT; i += 4 ) {
            ymm[i + 0] = _mm256_unpacklo_epi16( ymm_t[i + 0], ymm_t[i + 2] );
            ymm[i + 1] = _mm256_unpackhi_epi16( ymm_t[i + 0], ymm_t[i + 2] );
            ymm[i + 2] = _mm256_unpacklo_epi16( ymm_t[i + 1], ymm_t[i + 3] );
            ymm[i + 3] = _mm256_unpackhi_epi16( ymm_t[i + 1], ymm_t[i + 3] );
        }

        for( int i = 0; i < CHANNELS_8_BIT; i += 8 ) {
            ymm_t[i + 0] = _mm256_unpacklo_epi32( ymm[i + 0], ymm[i + 4] );
            ymm_t[i + 1] = _mm256_unpackhi_epi32( ymm[i + 0], ymm[i + 4] );
            ymm_t[i + 2] = _mm256_unpacklo_epi32( ymm[i + 1], ymm[i + 5] );
            ymm_t[i + 3] = _mm256_unpackhi_epi32( ymm[i + 1], ymm[i + 5] );
            ymm_t[i + 4] = _mm256_unpacklo_epi32( ymm[i + 2], ymm[i + 6] );
            ymm_t[i + 5] = _mm256_unpackhi_epi32( ymm[i + 2], ymm[i + 6] );
            ymm_t[i + 6] = _mm256_unpacklo_epi32( ymm[i + 3], ymm[i + 7] );
            ymm_t[i + 7] = _mm256_unpackhi_epi32( ymm[i + 3], ymm[i + 7] );
        }

        for( int i = 0; i < CHANNELS_8_BIT; i += 16 ) {
            ymm[i + 0] = _mm256_unpacklo_epi64( ymm_t[i + 0], ymm_t[i + 8] );
            ymm[i + 1] = _mm256_unpackhi_epi64( ymm_t[i + 0], ymm_t[i + 8] );
            ymm[i + 2] = _mm256_unpacklo_epi64( ymm_t[i + 1], ymm_t[i + 9] );
            ymm[i + 3] = _mm256_unpackhi_epi64( ymm_t[i + 1], ymm_t[i + 9] );
            ymm[i + 4] = _mm256_unpacklo_epi64( ymm_t[i + 2], ymm_t[i + 10] );
            ymm[i + 5] = _mm256_unpackhi_epi64( ymm_t[i + 2], ymm_t[i + 10] );
            ymm[i + 6] = _mm256_unpacklo_epi64( ymm_t[i + 3], ymm_t[i + 11] );
            ymm[i + 7] = _mm256_unpackhi_epi64( ymm_t[i + 3], ymm_t[i + 11] );
            ymm[i + 8] = _mm256_unpacklo_epi64( ymm_t[i + 4], ymm_t[i + 12] );
            ymm[i + 9] = _mm256_unpackhi_epi64( ymm_t[i + 4], ymm_t[i + 12] );
            ymm[i + 10] = _mm256_unpacklo_epi64( ymm_t[i + 5], ymm_t[i + 13] );
            ymm[i + 11] = _mm256_unpackhi_epi64( ymm_t[i + 5], ymm_t[i + 13] );
            ymm[i + 12] = _mm256_unpacklo_epi64( ymm_t[i + 6], ymm_t[i + 14] );
            ymm[i + 13] = _mm256_unpackhi_epi64( ymm_t[i + 6], ymm_t[i + 14] );
            ymm[i + 14] = _mm256_unpacklo_epi64( ymm_t[i + 7], ymm_t[i + 15] );
            ymm[i + 15] = _mm256_unpackhi_epi64( ymm_t[i + 7], ymm_t[i + 15] );
        }

        for( int i = 0; i < (CHANNELS_8_BIT / 2); i++ ) {
            ymm_t[i + 0]  = _mm256_permute2x128_si256( ymm[i + 0], ymm[i + 16], (2 << 4) | 0 );
            ymm_t[i + 16] = _mm256_permute2x128_si256( ymm[i + 0], ymm[i + 16], (3 << 4) | 1 );
        }

        // store matrix
        for( int i = 0; i < CHANNELS_8_BIT; i++ ) {
            _mm256_store_si256( (__m256i *) (dprofile + j * CHANNELS_8_BIT + i * 128), ymm_t[i] );
        }
    }
#if 0
    dbg_dprofile_dump_8( dprofile, CDEPTH_8_BIT, CHANNELS_8_BIT );
#endif
}
#else
void dprofile_fill_8_sse41( int8_t * dprofile, uint8_t * dseq_search_window ) {
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

    // 4 x 16 db symbols
    // ca (60x2+68x2)x4 = 976 instructions TODO verify these numbers

#if 0
    dbg_dumpscorematrix_8( score_matrix_7 );

    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_8_BIT_SSE; z++ )
            fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_8_BIT_SSE + z]] );
        fprintf( stderr, "\n" );
    }
#endif

    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        int d[CHANNELS_8_BIT_SSE];

        for( int i = 0; i < CHANNELS_8_BIT_SSE; i++ )
            d[i] = dseq_search_window[j * CHANNELS_8_BIT_SSE + i] << 5;

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

            _mm_store_si128( (__m128i *) (dprofile + CHANNELS_8_BIT_SSE * j + 64 * i + 0), xmm0 );
            _mm_store_si128( (__m128i *) (dprofile + CHANNELS_8_BIT_SSE * j + 64 * i + 64), xmm3 );
            _mm_store_si128( (__m128i *) (dprofile + CHANNELS_8_BIT_SSE * j + 64 * i + 128), xmm2 );
            _mm_store_si128( (__m128i *) (dprofile + CHANNELS_8_BIT_SSE * j + 64 * i + 192), xmm7 );
            _mm_store_si128( (__m128i *) (dprofile + CHANNELS_8_BIT_SSE * j + 64 * i + 256), xmm1 );
            _mm_store_si128( (__m128i *) (dprofile + CHANNELS_8_BIT_SSE * j + 64 * i + 320), xmm11 );
            _mm_store_si128( (__m128i *) (dprofile + CHANNELS_8_BIT_SSE * j + 64 * i + 384), xmm6 );
            _mm_store_si128( (__m128i *) (dprofile + CHANNELS_8_BIT_SSE * j + 64 * i + 448), xmm15 );
        }
    }
#if 0
    dbg_dprofile_dump_8( dprofile, CDEPTH_8_BIT, CHANNELS_8_BIT_SSE );
#endif
}
#endif
