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

static void search_8_avx2_init_query( p_s8info s, int q_count, seq_buffer * queries ) {
    s->q_count = q_count;

    s->maxqlen = 0;

    for( int i = 0; i < q_count; ++i ) {
        p_s8query query = (p_s8query) xmalloc( sizeof(struct s8query) );

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
            query->q_table_avx[j] = &s->dprofile_avx[CDEPTH_8_BIT * (int) (queries[i].seq.seq[j])];

        s->queries[i] = query;
    }

    if( s->hearray_avx )
        free( s->hearray_avx );
    s->hearray_avx = (__m256i *) xmalloc( 2 * s->maxqlen * sizeof(__m256i ) );
    memset( s->hearray_avx, 0, 2 * s->maxqlen * sizeof(__m256i ) );
}

void search_8_avx2_init( p_search_data sdp, p_s8info s ) {
    s->dprofile_avx = (__m256i *) xmalloc( sizeof(int8_t) * CDEPTH_8_BIT * CHANNELS_8_BIT_AVX * SCORE_MATRIX_DIM );

    search_8_avx2_init_query( s, sdp->q_count, sdp->queries );
}

void dprofile_fill_8_avx2( int8_t * dprofile, uint8_t * dseq_search_window ) {
    __m256i ymm[CHANNELS_8_BIT_AVX];
    __m256i ymm_t[CHANNELS_8_BIT_AVX];

    // 4 x 16 db symbols
    // ca (60x2+68x2)x4 = 976 instructions TODO verify these numbers

#if 0
     dbg_dumpscorematrix_8( score_matrix_7 );

    printf( "DB search window:\n");
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_8_BIT_AVX; z++ )
        fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_8_BIT_AVX + z]] );
        fprintf( stderr, "\n" );
    }
#endif
    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        int d[CHANNELS_8_BIT_AVX];

        for( int i = 0; i < CHANNELS_8_BIT_AVX; i++ ) // TODO try out to merge this and the next for loop. Does it save cycles?
            d[i] = dseq_search_window[j * CHANNELS_8_BIT_AVX + i] << 5;

        // load matrix
        for( int i = 0; i < CHANNELS_8_BIT_AVX; i++ ) {
            ymm[i] = _mm256_lddqu_si256( (__m256i *) (score_matrix_7 + d[i]) );
        }

        // transpose matrix
        for( int i = 0; i < CHANNELS_8_BIT_AVX; i += 2 ) {
            ymm_t[i + 0] = _mm256_unpacklo_epi8( ymm[i + 0], ymm[i + 1] );
            ymm_t[i + 1] = _mm256_unpackhi_epi8( ymm[i + 0], ymm[i + 1] );
        }

        for( int i = 0; i < CHANNELS_8_BIT_AVX; i += 4 ) {
            ymm[i + 0] = _mm256_unpacklo_epi16( ymm_t[i + 0], ymm_t[i + 2] );
            ymm[i + 1] = _mm256_unpackhi_epi16( ymm_t[i + 0], ymm_t[i + 2] );
            ymm[i + 2] = _mm256_unpacklo_epi16( ymm_t[i + 1], ymm_t[i + 3] );
            ymm[i + 3] = _mm256_unpackhi_epi16( ymm_t[i + 1], ymm_t[i + 3] );
        }

        for( int i = 0; i < CHANNELS_8_BIT_AVX; i += 8 ) {
            ymm_t[i + 0] = _mm256_unpacklo_epi32( ymm[i + 0], ymm[i + 4] );
            ymm_t[i + 1] = _mm256_unpackhi_epi32( ymm[i + 0], ymm[i + 4] );
            ymm_t[i + 2] = _mm256_unpacklo_epi32( ymm[i + 1], ymm[i + 5] );
            ymm_t[i + 3] = _mm256_unpackhi_epi32( ymm[i + 1], ymm[i + 5] );
            ymm_t[i + 4] = _mm256_unpacklo_epi32( ymm[i + 2], ymm[i + 6] );
            ymm_t[i + 5] = _mm256_unpackhi_epi32( ymm[i + 2], ymm[i + 6] );
            ymm_t[i + 6] = _mm256_unpacklo_epi32( ymm[i + 3], ymm[i + 7] );
            ymm_t[i + 7] = _mm256_unpackhi_epi32( ymm[i + 3], ymm[i + 7] );
        }

        for( int i = 0; i < CHANNELS_8_BIT_AVX; i += 16 ) {
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

        for( int i = 0; i < (CHANNELS_8_BIT_AVX / 2); i++ ) {
            ymm_t[i + 0]  = _mm256_permute2x128_si256( ymm[i + 0], ymm[i + 16], (2 << 4) | 0 );
            ymm_t[i + 16] = _mm256_permute2x128_si256( ymm[i + 0], ymm[i + 16], (3 << 4) | 1 );
        }

        // store matrix
        for( int i = 0; i < CHANNELS_8_BIT_AVX; i++ ) {
//                _mm256_stream_si256 prevents caching              TODO choose one
//                _mm256_store_si256 does not prevent caching

            _mm256_store_si256( (__m256i *) (dprofile + j * CHANNELS_8_BIT_AVX + i * 128), ymm_t[i] );
        }
    }
#if 0
    dbg_dprofile_dump_8( dprofile, CDEPTH_8_BIT, CHANNELS_8_BIT_AVX );
#endif
}
