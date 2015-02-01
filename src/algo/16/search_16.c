/*
 * search_16.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "search_16.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../searcher.h"
#include "../63/search_63.h"
#include "../../util/minheap.h"
#include "../../util/linked_list.h"
#include "../../util/util.h"
#include "../../db_iterator.h"
#include "../../matrices.h"

static void (*search_algo)( p_s16info, p_db_chunk, p_minheap, p_node *, int );

void search16_init_algo( int search_type ) {
    if( search_type == SMITH_WATERMAN ) {
        search_algo = &search_16_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        search_algo = &search_16_nw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH_SELLERS ) {
//        search_algo = &search16_nw_sellers; TODO not yet implemented
        ffatal( "\nnot yet implemented\n\n" );
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }
}

static void free_query( p_s16query query ) {
    if( query ) {
        free( query->q_table );
        query->q_len = 0;

        free( query );
    }
}

void search16_exit( p_s16info s ) {
    /* free mem for dprofile, hearray, dir, qtable */
    if( s->hearray )
        free( s->hearray );
    if( s->dprofile )
        free( s->dprofile );

    for( int i = 0; i < s->q_count; i++ ) {
        free_query( s->queries[i] );
    }
    s->q_count = 0;

    free( s );
}

static void search16_init_query( p_s16info s, int q_count, seq_buffer * queries ) {
    s->q_count = q_count;

    s->maxqlen = 0;

    for( int i = 0; i < q_count; ++i ) {
        if( s->queries[i] )
            free_query( s->queries[i] );

        p_s16query query = (p_s16query) xmalloc( sizeof(struct s16query) );

        query->q_len = queries[i].seq.len;
        query->seq = queries[i].seq.seq;

        if( query->q_len > s->maxqlen ) {
            s->maxqlen = query->q_len;
        }

        query->q_table = (__m128i **) xmalloc( query->q_len * sizeof(__m128i *) );

        for( int j = 0; j < query->q_len; j++ )
            /*
             * q_table holds pointers to dprofile, which holds the actual query data.
             * The dprofile is filled during the search for every four columns, that are searched.
             */

            query->q_table[j] = &s->dprofile[ CDEPTH_16_BIT * (int) (queries[i].seq.seq[j])];

        s->queries[i] = query;
    }

    if( s->hearray )
        free( s->hearray );
    s->hearray = (__m128i *) xmalloc( 2 * s->maxqlen * sizeof(__m128i ) );
    memset( s->hearray, 0, 2 * s->maxqlen * sizeof(__m128i ) );
}

void dprofile_fill16( int16_t * dprofile, uint8_t * dseq ) {
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

    /* does not require ssse3 */
    /* approx 4*(5*8+2*40)=480 instructions */

#if 0
    dumpscorematrix( score_matrix_16 );

    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        for( int z = 0; z < CHANNELS_16_BIT; z++ )
            fprintf( stderr, " [%c]", sym_ncbi_nt16u[dseq[j * CHANNELS_16_BIT + z]] );
        fprintf( stderr, "\n" );
    }
#endif

    for( int j = 0; j < CDEPTH_16_BIT; j++ ) {
        int d[CHANNELS_16_BIT];
        for( int z = 0; z < CHANNELS_16_BIT; z++ )
            d[z] = dseq[j * CHANNELS_16_BIT + z] << 5;

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

            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 0) + CHANNELS_16_BIT * j),
                    xmm8 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 1) + CHANNELS_16_BIT * j),
                    xmm9 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 2) + CHANNELS_16_BIT * j),
                    xmm10 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 3) + CHANNELS_16_BIT * j),
                    xmm11 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 4) + CHANNELS_16_BIT * j),
                    xmm12 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 5) + CHANNELS_16_BIT * j),
                    xmm13 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 6) + CHANNELS_16_BIT * j),
                    xmm14 );
            _mm_store_si128( (__m128i *) (dprofile + CDEPTH_16_BIT * CHANNELS_16_BIT * (i + 7) + CHANNELS_16_BIT * j),
                    xmm15 );
        }
    }
#if 0
    dprofile_dump16( dprofile );
#endif
}

unsigned long search_16_chunk( p_s16info s16info, p_minheap heap, p_db_chunk chunk, p_search_data sdp ) {
    unsigned long searches_done = 0;

    p_node overflow_list = 0;

    for( int q_id = 0; q_id < sdp->q_count; q_id++ ) {
        search_algo( s16info, chunk, heap, &overflow_list, q_id );

        searches_done += chunk->fill_pointer;
    }

    if( overflow_list ) {
        p_search_result res = xmalloc( sizeof(struct search_result) );
        res->heap = heap;
        res->chunk_count = 0;
        res->seq_count = 0;

        search_63( convert_to_chunk( overflow_list ), sdp, res );
    }

    return searches_done;
}

p_s16info search_16_init( p_search_data sdp ) {
    /* prepare alloc of qtable, dprofile, hearray, dir */
    p_s16info s = (p_s16info) xmalloc( sizeof(struct s16info) );

    s->dprofile = (__m128i *) xmalloc( sizeof(int16_t) * CDEPTH_16_BIT * CHANNELS_16_BIT * SCORE_MATRIX_DIM );
    s->q_count = 0;
    s->hearray = 0;
    s->maxdlen = ssa_db_get_longest_sequence();

    for( int i = 0; i < 6; i++ ) {
        s->queries[i] = 0;
    }

    s->penalty_gap_open = gapO;
    s->penalty_gap_extension = gapE;

    search16_init_query( s, sdp->q_count, sdp->queries );

    return s;
}

void search_16( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo ) {
        ffatal( "\n 16 bit search not initialized!!\n\n" );
    }

    p_s16info s16info = search_16_init( sdp );

    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        int searched_sequences = search_16_chunk( s16info, res->heap, chunk, sdp );

        assert( searched_sequences == chunk->fill_pointer * sdp->q_count );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    search16_exit( s16info );
}
