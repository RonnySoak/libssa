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
#include <assert.h>

#include "../searcher.h"
#include "../../util/util.h"
#include "../../util/linked_list.h"
#include "../../db_iterator.h"
#include "../../matrices.h"
#include "../../cpu_config.h"
#include "../16/search_16.h"

static void (*search_algo)( p_s8info, p_db_chunk, p_minheap, p_node *, int );

void search_8_init_algo( int search_type ) {
    if( !is_sse2_enabled() ) {
        ffatal( "\nAVX2 and SSE4.1 not enabled. No 8 bit search possible\n\n", search_type );
    }

    if( search_type == SMITH_WATERMAN ) {
        if( is_avx2_enabled() ) {
            search_algo = &search_8_avx2_sw;
        }
        else if( is_sse41_enabled() ) {
            search_algo = &search_8_sse41_sw;
        }
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        if( is_avx2_enabled() ) {
            search_algo = &search_8_avx2_nw;
        }
        else if( is_sse41_enabled() ) {
            search_algo = &search_8_sse41_nw;
        }
    }
    else if( search_type == NEEDLEMAN_WUNSCH_SELLERS ) {
//        search_algo = &search8_nw_sellers; TODO not yet implemented
        ffatal( "\nnot yet implemented\n\n" );
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }

}

p_s8info search_8_init( p_search_data sdp ) {
    p_s8info s = (p_s8info) xmalloc( sizeof(struct s8info) );

    s->dprofile_avx = 0;
    s->dprofile_sse = 0;
    s->hearray_avx = 0;
    s->hearray_sse = 0;

    s->maxdlen = ssa_db_get_longest_sequence();

    s->q_count = 0;
    for( int i = 0; i < 6; i++ ) {
        s->queries[i] = 0;
    }

    s->penalty_gap_open = gapO;
    s->penalty_gap_extension = gapE;

    s->s16info = 0;

    if( is_avx2_enabled() ) {
        search_8_avx2_init( sdp, s );
    }
    else {
        search_8_sse41_init( sdp, s );
    }

    return s;
}

void search_8_exit( p_s8info s ) {
    /* free mem for dprofile, hearray, dir, qtable */
    if( s->hearray_avx )
        free( s->hearray_avx );
    if( s->hearray_sse )
        free( s->hearray_sse );
    if( s->dprofile_sse )
        free( s->dprofile_sse );
    if( s->dprofile_avx )
        free( s->dprofile_avx );

    for( int i = 0; i < s->q_count; i++ ) {
        if( s->queries[i]->q_table_sse )
            free( s->queries[i]->q_table_sse );
        if( s->queries[i]->q_table_avx )
            free( s->queries[i]->q_table_avx );

        s->queries[i]->q_len = 0;

        free( s->queries[i] );
        s->queries[i] = 0;
    }
    s->q_count = 0;

    if( s->s16info ) {
        search_16_exit( s->s16info );
        s->s16info = 0;
    }

    free( s );
}

static unsigned long search_8_chunk( p_s8info s8info, p_minheap heap, p_db_chunk chunk, p_search_data sdp ) {
    unsigned long searches_done = 0;

    p_node overflow_list = 0;

    for( int q_id = 0; q_id < sdp->q_count; q_id++ ) {
        search_algo( s8info, chunk, heap, &overflow_list, q_id );

        searches_done += chunk->fill_pointer;
    }

    if( overflow_list ) {
        printf( "Overflow detected: re-aligning %ld sequences with 16 bit\n", ll_size( overflow_list ) ); // TODO show info or not?

        /*
         * XXX
         * Re-aligning sequences with 16 bit might result in a different order of the sequences in the heap.
         * If a new element to the heap has a score equal to the score of the lowest element, in the heap,
         * the new element is omitted.
         * This behavior of the heap might result in slightly different result for the 16 bit search and the
         * 8 bit search with re-aligned sequences. Although the different results mean only, that another
         * DB sequence, with the same score, is show, when comparing the results for the different bit-widths
         * for searches.
         *
         * TODO decide to keep or remove this non-deterministic behavior
         */
        if( !s8info->s16info ) {
            s8info->s16info = search_16_init( sdp );
        }

        p_db_chunk overflow_chunk = convert_to_chunk( overflow_list );

        search_16_chunk( s8info->s16info, heap, overflow_chunk, sdp );

        free( overflow_chunk->seq );
        free( overflow_chunk );
    }

    return searches_done;
}

void search_8( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo ) {
        ffatal( "\n 8 bit search not initialized.\n" );
    }

    p_s8info s8info = search_8_init( sdp );

    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        int searched_sequences = search_8_chunk( s8info, res->heap, chunk, sdp );

        assert( searched_sequences == chunk->fill_pointer * sdp->q_count );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    search_8_exit( s8info );
}
