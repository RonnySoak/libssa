/*
 * search_8.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../searcher.h"
#include "../../util/minheap.h"
#include "../../util/util.h"
#include "../../db_iterator.h"
#include "../../matrices.h"
#include "search_8.h"

static void (*search_algo)( p_s8info, p_db_chunk, p_minheap, int );

void search8_init_algo( int search_type ) {
    if( search_type == SMITH_WATERMAN ) {
        search_algo = &search_8_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        search_algo = &search_8_nw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH_SELLERS ) {
//        search_algo = &search8_nw_sellers; TODO not yet implemented
        ffatal( "\nnot yet implemented\n\n" );
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }
}

static p_s8info search8_init() {
    /* prepare alloc of qtable, dprofile, hearray, dir */
    p_s8info s = (p_s8info) xmalloc( sizeof(struct s8info) );

    s->dprofile = (__m128i *) xmalloc( sizeof(int8_t) * CDEPTH_8_BIT * CHANNELS_8_BIT * SCORE_MATRIX_DIM );
    s->q_count = 0;
    s->hearray = 0;
    s->maxdlen = ssa_db_get_longest_sequence();

    for( int i = 0; i < 6; i++ ) {
        s->queries[i] = 0;
    }

    s->penalty_gap_open = gapO;
    s->penalty_gap_extension = gapE;

    return s;
}

static void free_query( p_s8query query ) {
    if( query ) {
        free( query->q_table );
        query->q_len = 0;
        query = 0;
    }
}

static void search8_exit( p_s8info s ) {
    /* free mem for dprofile, hearray, dir, qtable */
    if( s->hearray )
        free( s->hearray );
    if( s->dprofile )
        free( s->dprofile );

    for( int i = 0; i < s->q_count; i++ ) {
        free_query( s->queries[i] );
    }

    free( s );
}

static void search8_init_query( p_s8info s, int q_count, seq_buffer * queries ) {
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

        query->q_table = (__m128i **) xmalloc( query->q_len * sizeof(__m128i *) );

        for( int j = 0; j < query->q_len; j++ )
            /*
             * q_table holds pointers to dprofile, which holds the actual query data.
             * The dprofile is filled during the search for every four columns, that are searched.
             */
            query->q_table[j] = s->dprofile + CDEPTH_8_BIT * (int) (queries[i].seq.seq[j]);

        s->queries[i] = query;
    }

    if( s->hearray )
        free( s->hearray );
    s->hearray = (__m128i *) xmalloc( 2 * s->maxqlen * sizeof(__m128i ) );
    memset( s->hearray, 0, 2 * s->maxqlen * sizeof(__m128i ) );
}

void dprofile_fill8( int8_t * dprofile, uint8_t * dseq ) {
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;

    // 4 x 16 db symbols
    // ca (60x2+68x2)x4 = 976 instructions

    for( int j = 0; j < CDEPTH_8_BIT; j++ ) {
        unsigned d[CHANNELS_8_BIT];

        for( int i = 0; i < CHANNELS_8_BIT; i++ )
            d[i] = dseq[j * CHANNELS_8_BIT + i] << 5;

        xmm0 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + d[0]) );
        xmm2 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + d[2]) );
        xmm4 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + d[4]) );
        xmm6 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + d[6]) );
        xmm8 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + d[8]) );
        xmm10 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + d[10]) );
        xmm12 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + d[12]) );
        xmm14 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + d[14]) );

        xmm0 = _mm_unpacklo_epi8( xmm0, *(__m128i *) (score_matrix_7 + d[1]) );
        xmm2 = _mm_unpacklo_epi8( xmm2, *(__m128i *) (score_matrix_7 + d[3]) );
        xmm4 = _mm_unpacklo_epi8( xmm4, *(__m128i *) (score_matrix_7 + d[5]) );
        xmm6 = _mm_unpacklo_epi8( xmm6, *(__m128i *) (score_matrix_7 + d[7]) );
        xmm8 = _mm_unpacklo_epi8( xmm8, *(__m128i *) (score_matrix_7 + d[9]) );
        xmm10 = _mm_unpacklo_epi8( xmm10, *(__m128i *) (score_matrix_7 + d[11]) );
        xmm12 = _mm_unpacklo_epi8( xmm12, *(__m128i *) (score_matrix_7 + d[13]) );
        xmm14 = _mm_unpacklo_epi8( xmm14, *(__m128i *) (score_matrix_7 + d[15]) );

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

        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 0), xmm0 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 64), xmm3 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 128), xmm2 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 192), xmm7 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 256), xmm1 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 320), xmm11 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 384), xmm6 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 448), xmm15 );

        // loads not aligned on 16 byte boundary, cannot load and unpack in one instr.

        xmm0 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[0]) );
        xmm1 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[1]) );
        xmm2 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[2]) );
        xmm3 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[3]) );
        xmm4 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[4]) );
        xmm5 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[5]) );
        xmm6 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[6]) );
        xmm7 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[7]) );
        xmm8 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[8]) );
        xmm9 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[9]) );
        xmm10 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[10]) );
        xmm11 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[11]) );
        xmm12 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[12]) );
        xmm13 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[13]) );
        xmm14 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[14]) );
        xmm15 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 8 + d[15]) );

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

        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 512 + 0), xmm0 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 512 + 64), xmm3 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 512 + 128), xmm2 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 512 + 192), xmm7 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 512 + 256), xmm1 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 512 + 320), xmm11 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 512 + 384), xmm6 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 512 + 448), xmm15 );

        xmm0 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 16 + d[0]) );
        xmm2 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 16 + d[2]) );
        xmm4 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 16 + d[4]) );
        xmm6 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 16 + d[6]) );
        xmm8 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 16 + d[8]) );
        xmm10 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 16 + d[10]) );
        xmm12 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 16 + d[12]) );
        xmm14 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 16 + d[14]) );

        xmm0 = _mm_unpacklo_epi8( xmm0, *(__m128i *) (score_matrix_7 + 16 + d[1]) );
        xmm2 = _mm_unpacklo_epi8( xmm2, *(__m128i *) (score_matrix_7 + 16 + d[3]) );
        xmm4 = _mm_unpacklo_epi8( xmm4, *(__m128i *) (score_matrix_7 + 16 + d[5]) );
        xmm6 = _mm_unpacklo_epi8( xmm6, *(__m128i *) (score_matrix_7 + 16 + d[7]) );
        xmm8 = _mm_unpacklo_epi8( xmm8, *(__m128i *) (score_matrix_7 + 16 + d[9]) );
        xmm10 = _mm_unpacklo_epi8( xmm10, *(__m128i *) (score_matrix_7 + 16 + d[11]) );
        xmm12 = _mm_unpacklo_epi8( xmm12, *(__m128i *) (score_matrix_7 + 16 + d[13]) );
        xmm14 = _mm_unpacklo_epi8( xmm14, *(__m128i *) (score_matrix_7 + 16 + d[15]) );

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

        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1024 + 0), xmm0 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1024 + 64), xmm3 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1024 + 128), xmm2 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1024 + 192), xmm7 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1024 + 256), xmm1 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1024 + 320), xmm11 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1024 + 384), xmm6 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1024 + 448), xmm15 );

        // loads not aligned on 16 byte boundary, cannot load and unpack in one instr.

        xmm0 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[0]) );
        xmm1 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[1]) );
        xmm2 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[2]) );
        xmm3 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[3]) );
        xmm4 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[4]) );
        xmm5 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[5]) );
        xmm6 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[6]) );
        xmm7 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[7]) );
        xmm8 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[8]) );
        xmm9 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[9]) );
        xmm10 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[10]) );
        xmm11 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[11]) );
        xmm12 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[12]) );
        xmm13 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[13]) );
        xmm14 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[14]) );
        xmm15 = _mm_loadl_epi64( (__m128i *) (score_matrix_7 + 24 + d[15]) );

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

        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1536 + 0), xmm0 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1536 + 64), xmm3 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1536 + 128), xmm2 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1536 + 192), xmm7 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1536 + 256), xmm1 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1536 + 320), xmm11 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1536 + 384), xmm6 );
        _mm_store_si128( (__m128i *) (dprofile + 16 * j + 1536 + 448), xmm15 );
    }
}

static unsigned long search_chunk( p_s8info s8info, p_minheap heap, p_db_chunk chunk, p_search_data sdp ) {
    unsigned long searches_done = 0;

    for( int q_id = 0; q_id < sdp->q_count; q_id++ ) {
        search_algo( s8info, chunk, heap, q_id );

        searches_done += chunk->fill_pointer;
    }

    return searches_done;
}

void search_8( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo ) {
        ffatal( "\n 8 bit search not initialized!!\n\n" );
    }

    p_s8info s8info = search8_init();

    search8_init_query( s8info, sdp->q_count, sdp->queries );

    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        int searched_sequences = search_chunk( s8info, res->heap, chunk, sdp );

        assert( searched_sequences == chunk->fill_pointer * sdp->q_count );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    search8_exit( s8info );
}
