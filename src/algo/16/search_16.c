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
#include "../../util/minheap.h"
#include "../../util/util.h"
#include "../../db_iterator.h"
#include "../../matrices.h"

static void (*search_algo)( p_s16info, p_db_chunk, p_minheap, int );

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

p_s16info search16_init( int16_t penalty_gap_open, int16_t penalty_gap_extension ) {
    /* prepare alloc of qtable, dprofile, hearray, dir */
    p_s16info s = (p_s16info) xmalloc( sizeof(struct s16info) );

    s->maxdlen = 4 * ((ssa_db_get_longest_sequence() + 3) / 4);
    s->dprofile = (__m128i *) xmalloc( 2 * 4 * 8 * 16 );
    s->q_count = 0;
    s->hearray = 0;

    for( int i = 0; i < 16; i++ ) {
        for( int j = 0; j < 16; j++ ) {
            ((int16_t*) (&s->matrix))[16 * i + j] = SCORE_MATRIX_16( i, j );
        }
    }

    for( int i = 0; i < 6; i++ ) {
        s->queries[i] = 0;
    }

    s->penalty_gap_open = penalty_gap_open;
    s->penalty_gap_extension = penalty_gap_extension;

    return s;
}

static void free_query( p_s16query query ) {
    if( query ) {
        free( query->q_table );
        query->q_len = 0;
        query = 0;
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

    free( s );
}

void search16_init_query( p_s16info s, int q_count, seq_buffer * queries ) {
    s->q_count = q_count;

    uint64_t maxqlen = 0;

    for( int i = 0; i < q_count; ++i ) {
        if( s->queries[i] )
            free_query( s->queries[i] );

        p_s16query query = (p_s16query) xmalloc( sizeof(struct s16query) );

        query->q_len = queries[i].seq.len;

        if( query->q_len > maxqlen ) {
            maxqlen = query->q_len;
        }

        query->q_table = (__m128i **) xmalloc( query->q_len * sizeof(__m128i *) );

        for( int i = 0; i < query->q_len; i++ )
            /*
             * q_table holds pointers to dprofile, which holds the actual query data.
             * The dprofile is filled during the search for every four columns, that are searched.
             */
            query->q_table[i] = s->dprofile + 4 * (int) (queries->seq.seq[i]);

        s->queries[i] = query;
    }

    if( s->hearray )
        free( s->hearray );
    s->hearray = (__m128i *) xmalloc( 2 * maxqlen * sizeof(__m128i ) );
    memset( s->hearray, 0, 2 * maxqlen * sizeof(__m128i ) );
}

static unsigned long search_chunk( p_s16info s16info, p_minheap heap, p_db_chunk chunk, p_search_data sdp ) {
    unsigned long searches_done = 0;

    for( int q_id = 0; q_id < sdp->q_count; q_id++ ) {
        search_algo( s16info, chunk, heap, q_id );

        searches_done += chunk->fill_pointer;
    }

    return searches_done;
}

void search_16( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo ) {
        ffatal( "\n 16 bit search not initialized!!\n\n" );
    }

    p_s16info s16info = search16_init( gapO, gapE );

    search16_init_query( s16info, sdp->q_count, sdp->queries );

    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        int searched_sequences = search_chunk( s16info, res->heap, chunk, sdp );

        assert( searched_sequences == chunk->fill_pointer * sdp->q_count );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    search16_exit( s16info );
}
