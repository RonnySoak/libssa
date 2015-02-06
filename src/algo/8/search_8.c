/*
 * search_8.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "search_8.h"

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

static p_s8info (*init_search_env)( p_search_data );
static void (*exit_search_env)( p_s8info );

void search_8_init( int search_type ) {
    if( search_type == SMITH_WATERMAN ) {
        search_algo = &search_8_sse41_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        search_algo = &search_8_sse41_nw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH_SELLERS ) {
//        search_algo = &search8_nw_sellers; TODO not yet implemented
        ffatal( "\nnot yet implemented\n\n" );
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }

    if( is_avx2_enabled() ) {
        init_search_env = &search_8_avx2_init;
        exit_search_env = &search_8_avx2_exit;
    }
    else if( is_sse41_enabled() ) {
        init_search_env = &search_8_sse41_init;
        exit_search_env = &search_8_sse41_exit;
    }
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
            s8info->s16info = search_16_sse2_init( sdp );
        }

        p_db_chunk overflow_chunk = convert_to_chunk( overflow_list );

        search_16_chunk( s8info->s16info, heap, overflow_chunk, sdp );

        free( overflow_chunk->seq );
        free( overflow_chunk );
    }

    return searches_done;
}

void search_8( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo || !init_search_env || !exit_search_env ) {
        ffatal( "\n 8 bit search not initialized. Compute capability must be at least SSE4.1!!\n" );
    }

    p_s8info s8info = init_search_env( sdp );

    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        int searched_sequences = search_8_chunk( s8info, res->heap, chunk, sdp );

        assert( searched_sequences == chunk->fill_pointer * sdp->q_count );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    exit_search_env( s8info );
}
