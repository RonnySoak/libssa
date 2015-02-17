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

#include "../../util/util.h"
#include "../../matrices.h"
#include "../search.h"

#ifdef __AVX2__

#define CHANNELS_16_BIT CHANNELS_16_BIT_AVX
typedef __m256i __mxxxi;

#else // SSE2

#define CHANNELS_16_BIT CHANNELS_16_BIT_SSE
typedef __m128i  __mxxxi;

#endif


static void search_16_init_query( p_s16info s, int q_count, seq_buffer * queries ) {
    s->q_count = q_count;

    s->maxqlen = 0;

    for( int i = 0; i < q_count; ++i ) {
        p_s16query query = (p_s16query) xmalloc( sizeof(struct s16query) );

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
            query->q_table[j] = &s->dprofile[ CDEPTH_16_BIT * (int) (queries[i].seq.seq[j])];

        s->queries[i] = query;
    }

    if( s->hearray )
        free( s->hearray );
    s->hearray = (__mxxxi *) xmalloc( 2 * s->maxqlen * sizeof(__mxxxi ) );
    memset( s->hearray, 0, 2 * s->maxqlen * sizeof(__mxxxi ) );
}

#ifdef __AVX2__
p_s16info search_16_avx2_init( p_search_data sdp ) {
#else
p_s16info search_16_sse2_init( p_search_data sdp ) {
#endif
    p_s16info s = (p_s16info) xmalloc( sizeof(struct s16info) );

    s->dprofile = 0;
    s->hearray = 0;
    s->hearray_64 = 0;

    s->q_count = 0;
    for( int i = 0; i < 6; i++ ) {
        s->queries[i] = 0;
    }

    s->penalty_gap_open = gapO;
    s->penalty_gap_extension = gapE;

    s->dprofile = (__mxxxi *) xmalloc( sizeof(int16_t) * CDEPTH_16_BIT * CHANNELS_16_BIT * SCORE_MATRIX_DIM );

    search_16_init_query( s, sdp->q_count, sdp->queries );

    return s;
}

/*
 * dseq_search_window: CDEPTH_16_BIT x 128 bit
 *  - contains CDEPTH_16_BIT symbols of each DB sequence in all channels
 *
 * dprofile: sizeof(int16_t) * CDEPTH_16_BIT * CHANNELS_16_BIT * SCORE_MATRIX_DIM
 *  - contains the values accessed by the pointers in s16info->qtable
 *  - for each symbol of the CDEPTH_16_BIT symbols of each DB sequence, it contains the
 *      corresponding score matrix line
 */
#ifdef __AVX2__
void dprofile_fill_16_avx2( int16_t * dprofile, uint8_t * dseq_search_window ) {
    __m256i ymm[CHANNELS_16_BIT];
    __m256i ymm_t[CHANNELS_16_BIT];

    // 4 x 16 db symbols
    // ca (60x2+68x2)x4 = 976 instructions TODO verify these numbers

#if 0
    dbg_dumpscorematrix_16( score_matrix_16 );

    outf( "DB search window:\n");
    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_16_BIT; z++ )
            fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_16_BIT + z]] );
        fprintf( stderr, "\n" );
    }
#endif

    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        int d[CHANNELS_16_BIT];

        for( int i = 0; i < CHANNELS_16_BIT; i++ )
            d[i] = dseq_search_window[j * CHANNELS_16_BIT + i] << 5;

        for( int i = 0; i < SCORE_MATRIX_DIM; i += 16 ) {
            // load matrix
            for( int x = 0; x < CHANNELS_16_BIT; x++ ) {
                ymm[x] = _mm256_load_si256( (__m256i *) (score_matrix_16 + d[x] + i) );
            }

            // transpose matrix
            for( int x = 0; x < CHANNELS_16_BIT; x += 2 ) {
                ymm_t[x + 0] = _mm256_unpacklo_epi16( ymm[x + 0], ymm[x + 1] );
                ymm_t[x + 1] = _mm256_unpackhi_epi16( ymm[x + 0], ymm[x + 1] );

            }

            for( int x = 0; x < CHANNELS_16_BIT; x += 4 ) {
                ymm[x + 0] = _mm256_unpacklo_epi32( ymm_t[x + 0], ymm_t[x + 2] );
                ymm[x + 1] = _mm256_unpackhi_epi32( ymm_t[x + 0], ymm_t[x + 2] );
                ymm[x + 2] = _mm256_unpacklo_epi32( ymm_t[x + 1], ymm_t[x + 3] );
                ymm[x + 3] = _mm256_unpackhi_epi32( ymm_t[x + 1], ymm_t[x + 3] );
            }

            for( int x = 0; x < CHANNELS_16_BIT; x += 8 ) {
                ymm_t[x + 0] = _mm256_unpacklo_epi64( ymm[x + 0], ymm[x + 4] );
                ymm_t[x + 1] = _mm256_unpackhi_epi64( ymm[x + 0], ymm[x + 4] );
                ymm_t[x + 2] = _mm256_unpacklo_epi64( ymm[x + 1], ymm[x + 5] );
                ymm_t[x + 3] = _mm256_unpackhi_epi64( ymm[x + 1], ymm[x + 5] );
                ymm_t[x + 4] = _mm256_unpacklo_epi64( ymm[x + 2], ymm[x + 6] );
                ymm_t[x + 5] = _mm256_unpackhi_epi64( ymm[x + 2], ymm[x + 6] );
                ymm_t[x + 6] = _mm256_unpacklo_epi64( ymm[x + 3], ymm[x + 7] );
                ymm_t[x + 7] = _mm256_unpackhi_epi64( ymm[x + 3], ymm[x + 7] );
            }

            for( int x = 0; x < (CHANNELS_16_BIT / 2); x++ ) {
                ymm[x + 0] = _mm256_permute2x128_si256( ymm_t[x + 0], ymm_t[x + 8], (2 << 4) | 0 );
                ymm[x + 8] = _mm256_permute2x128_si256( ymm_t[x + 0], ymm_t[x + 8], (3 << 4) | 1 );
            }

            // store matrix
            for( int x = 0; x < CHANNELS_16_BIT; x++ ) {
                /*
                 * TODO do further tests and report in thesis
                 *
                 * If we use _mm256_stream_si256 here instead of _mm256_store_si256, the runtime drops
                 * drastically.
                 * The amount of CPU-clock cycles, reported by perf, rises from 14K to 45K !!!
                 *
                 * One issue here might be, that _mm256_stream_si256 uses a non-temporal hint, to prevent caching.
                 * In our case, where we access the stored data almost immediately, this might not be, what we want.
                 *
                 * See: https://software.intel.com/en-us/node/524115
                 *
                 * Using _mm256_stream_load_si256, instead of _mm256_load_si256, while loading data into ymm registers,
                 * seems to make no difference. The clock cycles are about the same.
                 *
                 * Same applies to the 8 bit AVX2 code.
                 */
                _mm256_store_si256( (__m256i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + x) + CHANNELS_16_BIT * j), ymm[x] );
            }
        }
    }
#if 0
    dbg_dprofile_dump_16( dprofile, CDEPTH_16_BIT, CHANNELS_16_BIT );
#endif
}
#else
void dprofile_fill_16_sse2( int16_t * dprofile, uint8_t * dseq_search_window ) {
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

    /* does not require ssse3 */
    /* approx 4*(5*8+2*40)=480 instructions TODO verify these numbers */

#if 0
    dbg_dumpscorematrix( score_matrix_16 );

    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_16_BIT; z++ )
            fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_16_BIT + z]] );
        fprintf( stderr, "\n" );
    }
#endif

    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        int d[CHANNELS_16_BIT];

        for( int z = 0; z < CHANNELS_16_BIT; z++ )
            d[z] = dseq_search_window[j * CHANNELS_16_BIT + z] << 5;

        for( int i = 0; i < SCORE_MATRIX_DIM; i += 8 ) {
            xmm0 = _mm_load_si128( (__m128i *) (score_matrix_16 + d[0] + i) );
            xmm1 = _mm_load_si128( (__m128i *) (score_matrix_16 + d[1] + i) );
            xmm2 = _mm_load_si128( (__m128i *) (score_matrix_16 + d[2] + i) );
            xmm3 = _mm_load_si128( (__m128i *) (score_matrix_16 + d[3] + i) );
            xmm4 = _mm_load_si128( (__m128i *) (score_matrix_16 + d[4] + i) );
            xmm5 = _mm_load_si128( (__m128i *) (score_matrix_16 + d[5] + i) );
            xmm6 = _mm_load_si128( (__m128i *) (score_matrix_16 + d[6] + i) );
            xmm7 = _mm_load_si128( (__m128i *) (score_matrix_16 + d[7] + i) );

            xmm8 = _mm_unpacklo_epi16( xmm0, xmm1 );
            xmm9 = _mm_unpackhi_epi16( xmm0, xmm1 );
            xmm10 = _mm_unpacklo_epi16( xmm2, xmm3 );
            xmm11 = _mm_unpackhi_epi16( xmm2, xmm3 );
            xmm12 = _mm_unpacklo_epi16( xmm4, xmm5 );
            xmm13 = _mm_unpackhi_epi16( xmm4, xmm5 );
            xmm14 = _mm_unpacklo_epi16( xmm6, xmm7 );
            xmm15 = _mm_unpackhi_epi16( xmm6, xmm7 );

            xmm0 = _mm_unpacklo_epi32( xmm8, xmm10 );
            xmm1 = _mm_unpackhi_epi32( xmm8, xmm10 );
            xmm2 = _mm_unpacklo_epi32( xmm12, xmm14 );
            xmm3 = _mm_unpackhi_epi32( xmm12, xmm14 );
            xmm4 = _mm_unpacklo_epi32( xmm9, xmm11 );
            xmm5 = _mm_unpackhi_epi32( xmm9, xmm11 );
            xmm6 = _mm_unpacklo_epi32( xmm13, xmm15 );
            xmm7 = _mm_unpackhi_epi32( xmm13, xmm15 );

            xmm8 = _mm_unpacklo_epi64( xmm0, xmm2 );
            xmm9 = _mm_unpackhi_epi64( xmm0, xmm2 );
            xmm10 = _mm_unpacklo_epi64( xmm1, xmm3 );
            xmm11 = _mm_unpackhi_epi64( xmm1, xmm3 );
            xmm12 = _mm_unpacklo_epi64( xmm4, xmm6 );
            xmm13 = _mm_unpackhi_epi64( xmm4, xmm6 );
            xmm14 = _mm_unpacklo_epi64( xmm5, xmm7 );
            xmm15 = _mm_unpackhi_epi64( xmm5, xmm7 );

            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 0) + CHANNELS_16_BIT * j), xmm8 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 1) + CHANNELS_16_BIT * j), xmm9 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 2) + CHANNELS_16_BIT * j), xmm10 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 3) + CHANNELS_16_BIT * j), xmm11 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 4) + CHANNELS_16_BIT * j), xmm12 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 5) + CHANNELS_16_BIT * j), xmm13 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 6) + CHANNELS_16_BIT * j), xmm14 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 7) + CHANNELS_16_BIT * j), xmm15 );
        }
    }
#if 0
    dbg_dprofile_dump_16( dprofile, CDEPTH_16_BIT, CHANNELS_16_BIT_SSE );
#endif
}
#endif
