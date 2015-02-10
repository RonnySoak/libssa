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
#include "../../util/util_sequence.h" // TODO remove
#include "../../db_iterator.h"
#include "../../cpu_config.h"
#include "../../matrices.h"

static void search_16_sse2_init_query( p_s16info s, int q_count, seq_buffer * queries ) {
    s->q_count = q_count;

    s->maxqlen = 0;

    for( int i = 0; i < q_count; ++i ) {
        p_s16query query = (p_s16query) xmalloc( sizeof(struct s16query) );

        query->q_len = queries[i].seq.len;
        query->seq = queries[i].seq.seq;

        if( query->q_len > s->maxqlen ) {
            s->maxqlen = query->q_len;
        }

        query->q_table_avx = 0;
        query->q_table_sse = (__m128i **) xmalloc( query->q_len * sizeof(__m128i *) );

        for( int j = 0; j < query->q_len; j++ )
            /*
             * q_table holds pointers to dprofile, which holds the actual query data.
             * The dprofile is filled during the search for every four columns, that are searched.
             */
            query->q_table_sse[j] = &s->dprofile_sse[ CDEPTH_16_BIT * (int) (queries[i].seq.seq[j])];

        s->queries[i] = query;
    }

    if( s->hearray_sse )
        free( s->hearray_sse );
    s->hearray_sse = (__m128i *) xmalloc( 2 * s->maxqlen * sizeof(__m128i ) );
    memset( s->hearray_sse, 0, 2 * s->maxqlen * sizeof(__m128i ) );
}

void search_16_sse2_init( p_search_data sdp, p_s16info s ) {
    s->dprofile_sse = (__m128i *) xmalloc( sizeof(int16_t) * CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * SCORE_MATRIX_DIM );

    search_16_sse2_init_query( s, sdp->q_count, sdp->queries );
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
void dprofile_fill_16_sse2( int16_t * dprofile, uint8_t * dseq_search_window ) {
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

    /* does not require ssse3 */
    /* approx 4*(5*8+2*40)=480 instructions TODO verify these numbers */

#if 0
    dbg_dumpscorematrix( score_matrix_16 );

    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_16_BIT_SSE; z++ )
            fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq_search_window[j * CHANNELS_16_BIT_SSE + z]] );
        fprintf( stderr, "\n" );
    }
#endif

    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        int d[CHANNELS_16_BIT_SSE];

        for( int z = 0; z < CHANNELS_16_BIT_SSE; z++ )
            d[z] = dseq_search_window[j * CHANNELS_16_BIT_SSE + z] << 5;

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

            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * (i + 0) + CHANNELS_16_BIT_SSE * j), xmm8 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * (i + 1) + CHANNELS_16_BIT_SSE * j), xmm9 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * (i + 2) + CHANNELS_16_BIT_SSE * j), xmm10 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * (i + 3) + CHANNELS_16_BIT_SSE * j), xmm11 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * (i + 4) + CHANNELS_16_BIT_SSE * j), xmm12 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * (i + 5) + CHANNELS_16_BIT_SSE * j), xmm13 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * (i + 6) + CHANNELS_16_BIT_SSE * j), xmm14 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT_SSE * (i + 7) + CHANNELS_16_BIT_SSE * j), xmm15 );
        }
    }
#if 0
    dbg_dprofile_dump_16( dprofile, CDEPTH_16_BIT, CHANNELS_16_BIT_SSE );
#endif
}
