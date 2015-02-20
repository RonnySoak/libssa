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
#include "../gap_costs.h"

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

    s->hearray = (__mxxxi *) xmalloc( 2 * s->maxqlen * sizeof(__mxxxi ) );
    memset( s->hearray, 0, 2 * s->maxqlen * sizeof(__mxxxi ) );
}

#ifdef __AVX2__
p_s16info search_16_avx2_init( p_search_data sdp ) {
#else
p_s16info search_16_sse2_init( p_search_data sdp ) {
#endif
    p_s16info s = (p_s16info) xmalloc( sizeof(struct s16info) );

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
void dprofile_fill_16_avx2( __mxxxi * dprofile, uint8_t * dseq_search_window ) {
    __m256i ymm[CHANNELS_16_BIT];
    __m256i ymm_t[CHANNELS_16_BIT];

    /*
     * Approximately 4*(16*5+2*6*16)=1088 instructions.
     *
     * TODO:
     * Could be reduced to 4*(16*5+1*6*16)=704 instructions, if we would use a 16x16 matrix only.
     * So in case of nucleotide sequences, we could reduce the number of instructions here.
     */
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
                _mm256_store_si256( (dprofile + CDEPTH_16_BIT * (i + x) + j), ymm[x] );
            }
        }
    }
#if 0
    dbg_dprofile_dump_16( dprofile, CDEPTH_16_BIT, CHANNELS_16_BIT );
#endif
}
#else
void dprofile_fill_16_sse2( __mxxxi * dprofile, uint8_t * dseq_search_window ) {
    __m128i xmm[CHANNELS_16_BIT_SSE];
    __m128i xmm_t[CHANNELS_16_BIT_SSE];

    /*
     * Approximately 4*(8*5+4*5*8)=800 instructions.
     *
     * TODO:
     * Could be reduced to 4*(8*5+2*5*8)=480 instructions, if we would use a 16x16 matrix only.
     * So in case of nucleotide sequences, we could reduce the number of instructions here.
     */

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
            for( int x = 0; x < CHANNELS_16_BIT_SSE; ++x ) {
                xmm[x] = _mm_load_si128( (__m128i *) (score_matrix_16 + d[x] + i) );
            }

            for( int x = 0; x < CHANNELS_16_BIT_SSE; x += 2 ) {
                xmm_t[x + 0] = _mm_unpacklo_epi16( xmm[x + 0], xmm[x + 1] );
                xmm_t[x + 1] = _mm_unpackhi_epi16( xmm[x + 0], xmm[x + 1] );
            }

            for( int x = 0; x < CHANNELS_16_BIT_SSE; x += 4 ) {
                xmm[x + 0] = _mm_unpacklo_epi32( xmm_t[x + 0], xmm_t[x + 2] );
                xmm[x + 1] = _mm_unpackhi_epi32( xmm_t[x + 0], xmm_t[x + 2] );
                xmm[x + 2] = _mm_unpacklo_epi32( xmm_t[x + 1], xmm_t[x + 3] );
                xmm[x + 3] = _mm_unpackhi_epi32( xmm_t[x + 1], xmm_t[x + 3] );
            }

            for( int x = 0; x < (CHANNELS_16_BIT_SSE / 2); x++ ) {
                xmm_t[(x*2) + 0] = _mm_unpacklo_epi64( xmm[x + 0], xmm[x + 4] );
                xmm_t[(x*2) + 1] = _mm_unpackhi_epi64( xmm[x + 0], xmm[x + 4] );
            }

            for( int x = 0; x < CHANNELS_16_BIT_SSE; x++ ) {
                _mm_store_si128( (dprofile + CDEPTH_16_BIT * (i + x) + j), xmm_t[x] );
            }
        }
    }
#if 0
    dbg_dprofile_dump_16( (int16_t *)dprofile, CDEPTH_16_BIT, CHANNELS_16_BIT_SSE );
#endif
}
#endif
