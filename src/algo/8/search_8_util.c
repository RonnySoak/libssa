/*
 * search_8_util.c
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
#include "../gap_costs.h"

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
void dprofile_fill_8_avx2( __mxxxi * dprofile, uint16_t * dseq_search_window ) {
    __m256i ymm[CHANNELS_8_BIT];
    __m256i ymm_t[CHANNELS_8_BIT];

    // 4 x 16 db symbols
    // ca (60x2+68x2)x4 = 976 instructions TODO verify these numbers

    /*
     * Approximately 4*(6*32+6*32)=1536 instructions.
     *
     * TODO:
     * This one is not so easy to reduce to an 16x16 matrix for nucleotides.
     * This requires a separate implementation of the inner loop.
     *
     * It might be possible to load two depth at once and transform them in one
     * iteration, and hence saving half the depth-iterations.
     */

#if 0
     dbg_dumpscorematrix_8( score_matrix_8 );

     outf( "DB search window:\n");
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_8_BIT; z++ )
        fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_8_BIT + z]] );
        fprintf( stderr, "\n" );
    }
#endif
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        // load matrix
        union {
            __m256i v[2];
            int16_t a[CHANNELS_8_BIT];
        } d;

        __m256i tmp = _mm256_loadu_si256( (__m256i *) (dseq_search_window + (j * CHANNELS_8_BIT)) );
        _mm256_store_si256( &d.v[0], _mm256_slli_epi16( tmp, 5 ) );

        tmp = _mm256_loadu_si256( (__m256i *) (dseq_search_window + (j * CHANNELS_8_BIT + (CHANNELS_8_BIT / 2))) );
        _mm256_store_si256( &d.v[1], _mm256_slli_epi16( tmp, 5 ) );

        for( int i = 0; i < CHANNELS_8_BIT; i++ ) {
            ymm[i] = _mm256_load_si256( (__m256i *) (score_matrix_8 + d.a[i]) );
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
            _mm256_store_si256( (dprofile + i * CDEPTH_8_BIT + j), ymm_t[i] );
        }
    }
#if 0
    dbg_dprofile_dump_8( dprofile, CDEPTH_8_BIT, CHANNELS_8_BIT );
#endif
}
#else
void dprofile_fill_8_sse41( __mxxxi * dprofile, uint16_t * dseq_search_window ) {
    __m128i xmm[CHANNELS_8_BIT];
    __m128i xmm_t[CHANNELS_8_BIT];

    /*
     * Approximately 4*(16*5+2*6*16)=1088 instructions.
     *
     * TODO:
     * Could be reduced to 4*(16*5+1*6*16)=704 instructions, if we would use a 16x16 matrix only.
     * So in case of nucleotide sequences, we could reduce the number of instructions here.
     *
     * TODO check assembly before and after optimization -> changing SCORE_MATRIX_DIM into a variable,
     * instead of a constant might change the loop unrolling to be less optimal ...
     */

#if 0
    dbg_dumpscorematrix_8( score_matrix_8 );

    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_8_BIT_SSE; z++ )
            fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_8_BIT_SSE + z]] );
        fprintf( stderr, "\n" );
    }
#endif

    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        union {
            __m128i v[2];
            int16_t a[CHANNELS_8_BIT];
        } d;

        __m128i tmp = _mm_loadu_si128( (__m128i *) (dseq_search_window + (j * CHANNELS_8_BIT)) );
        _mm_store_si128( &d.v[0], _mm_slli_epi16( tmp, 5 ) );

        tmp = _mm_loadu_si128( (__m128i *) (dseq_search_window + (j * CHANNELS_8_BIT + (CHANNELS_8_BIT / 2))) );
        _mm_store_si128( &d.v[1], _mm_slli_epi16( tmp, 5 ) );

        for( int i = 0; i < SCORE_MATRIX_DIM; i += 16 ) {
            // load matrix
            for( int x = 0; x < CHANNELS_8_BIT; x++ ) {
                xmm[x] = _mm_load_si128( (__m128i *) (score_matrix_8 + d.a[x] + i) );
            }

            // transpose matrix
            for( int x = 0; x < CHANNELS_8_BIT; x += 2 ) {
                xmm_t[x + 0] = _mm_unpacklo_epi8( xmm[x + 0], xmm[x + 1] );
                xmm_t[x + 1] = _mm_unpackhi_epi8( xmm[x + 0], xmm[x + 1] );
            }

            for( int x = 0; x < CHANNELS_8_BIT; x += 4 ) {
                xmm[x + 0] = _mm_unpacklo_epi16( xmm_t[x + 0], xmm_t[x + 2] );
                xmm[x + 1] = _mm_unpackhi_epi16( xmm_t[x + 0], xmm_t[x + 2] );
                xmm[x + 2] = _mm_unpacklo_epi16( xmm_t[x + 1], xmm_t[x + 3] );
                xmm[x + 3] = _mm_unpackhi_epi16( xmm_t[x + 1], xmm_t[x + 3] );
            }

            for( int x = 0; x < CHANNELS_8_BIT; x += 8 ) {
                xmm_t[x + 0] = _mm_unpacklo_epi32( xmm[x + 0], xmm[x + 4] );
                xmm_t[x + 1] = _mm_unpackhi_epi32( xmm[x + 0], xmm[x + 4] );
                xmm_t[x + 2] = _mm_unpacklo_epi32( xmm[x + 1], xmm[x + 5] );
                xmm_t[x + 3] = _mm_unpackhi_epi32( xmm[x + 1], xmm[x + 5] );
                xmm_t[x + 4] = _mm_unpacklo_epi32( xmm[x + 2], xmm[x + 6] );
                xmm_t[x + 5] = _mm_unpackhi_epi32( xmm[x + 2], xmm[x + 6] );
                xmm_t[x + 6] = _mm_unpacklo_epi32( xmm[x + 3], xmm[x + 7] );
                xmm_t[x + 7] = _mm_unpackhi_epi32( xmm[x + 3], xmm[x + 7] );
            }

            for( int x = 0; x < (CHANNELS_8_BIT / 2); x++ ) {
                xmm[(x*2) + 0] = _mm_unpacklo_epi64( xmm_t[x + 0], xmm_t[x + 8] );
                xmm[(x*2) + 1] = _mm_unpackhi_epi64( xmm_t[x + 0], xmm_t[x + 8] );
            }

            // store matrix
            for( int x = 0; x < CHANNELS_8_BIT; x++ ) {
                _mm_store_si128( (dprofile + CDEPTH_8_BIT * (i + x) + j), xmm[x] );
            }
        }
    }
#if 0
    dbg_dprofile_dump_8( (int8_t *)dprofile, CDEPTH_8_BIT, CHANNELS_8_BIT_SSE );
#endif
}
#endif
