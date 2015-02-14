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

        for( size_t j = 0; j < query->q_len; j++ )
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

void dprofile_fill_16_avx2( int16_t * dprofile, uint8_t * dseq_search_window ) {
    __m256i ymm[CHANNELS_16_BIT_AVX];
    __m256i ymm_t[CHANNELS_16_BIT_AVX];

    // 4 x 16 db symbols
    // ca (60x2+68x2)x4 = 976 instructions TODO verify these numbers

#if 0
    dbg_dumpscorematrix_16( score_matrix_16 );

    outf( "DB search window:\n");
    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_16_BIT_AVX; z++ )
            fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_16_BIT_AVX + z]] );
        fprintf( stderr, "\n" );
    }
#endif

    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        int d[CHANNELS_16_BIT_AVX];

        for( int i = 0; i < CHANNELS_16_BIT_AVX; i++ )
            d[i] = dseq_search_window[j * CHANNELS_16_BIT_AVX + i] << 5;

        for( int i = 0; i < SCORE_MATRIX_DIM; i += 16 ) {
            // load matrix
            for( int x = 0; x < CHANNELS_16_BIT_AVX; x++ ) {
                ymm[x] = _mm256_load_si256( (__m256i *) (score_matrix_16 + d[x] + i) );
            }

            // transpose matrix
            for( int x = 0; x < CHANNELS_16_BIT_AVX; x += 2 ) {
                ymm_t[x + 0] = _mm256_unpacklo_epi16( ymm[x + 0], ymm[x + 1] );
                ymm_t[x + 1] = _mm256_unpackhi_epi16( ymm[x + 0], ymm[x + 1] );

            }

            for( int x = 0; x < CHANNELS_16_BIT_AVX; x += 4 ) {
                ymm[x + 0] = _mm256_unpacklo_epi32( ymm_t[x + 0], ymm_t[x + 2] );
                ymm[x + 1] = _mm256_unpackhi_epi32( ymm_t[x + 0], ymm_t[x + 2] );
                ymm[x + 2] = _mm256_unpacklo_epi32( ymm_t[x + 1], ymm_t[x + 3] );
                ymm[x + 3] = _mm256_unpackhi_epi32( ymm_t[x + 1], ymm_t[x + 3] );
            }

            for( int x = 0; x < CHANNELS_16_BIT_AVX; x += 8 ) {
                ymm_t[x + 0] = _mm256_unpacklo_epi64( ymm[x + 0], ymm[x + 4] );
                ymm_t[x + 1] = _mm256_unpackhi_epi64( ymm[x + 0], ymm[x + 4] );
                ymm_t[x + 2] = _mm256_unpacklo_epi64( ymm[x + 1], ymm[x + 5] );
                ymm_t[x + 3] = _mm256_unpackhi_epi64( ymm[x + 1], ymm[x + 5] );
                ymm_t[x + 4] = _mm256_unpacklo_epi64( ymm[x + 2], ymm[x + 6] );
                ymm_t[x + 5] = _mm256_unpackhi_epi64( ymm[x + 2], ymm[x + 6] );
                ymm_t[x + 6] = _mm256_unpacklo_epi64( ymm[x + 3], ymm[x + 7] );
                ymm_t[x + 7] = _mm256_unpackhi_epi64( ymm[x + 3], ymm[x + 7] );
            }

            for( int x = 0; x < (CHANNELS_16_BIT_AVX / 2); x++ ) {
                ymm[x + 0] = _mm256_permute2x128_si256( ymm_t[x + 0], ymm_t[x + 8], (2 << 4) | 0 );
                ymm[x + 8] = _mm256_permute2x128_si256( ymm_t[x + 0], ymm_t[x + 8], (3 << 4) | 1 );
            }

            // store matrix
            for( int x = 0; x < CHANNELS_16_BIT_AVX; x++ ) {
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
                _mm256_store_si256( (__m256i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_AVX * (i + x) + CHANNELS_16_BIT_AVX * j), ymm[x] );
            }
        }
    }
#if 0
    dbg_dprofile_dump_16( dprofile, CDEPTH_16_BIT, CHANNELS_16_BIT_AVX );
#endif
}
