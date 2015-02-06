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
#include "../../cpu_config.h"

static void (*search_algo)( p_s16info, p_db_chunk, p_minheap, p_node *, int );

static p_s16info (*init_search_env)( p_search_data );
static void (*exit_search_env)( p_s16info );

void search_16_init_algo( int search_type ) {
    if( search_type == SMITH_WATERMAN ) {
        search_algo = &search_16_sse2_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        search_algo = &search_16_sse2_nw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH_SELLERS ) {
//        search_algo = &search16_nw_sellers; TODO not yet implemented
        ffatal( "\nnot yet implemented\n\n" );
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }

    if( is_avx2_enabled() ) {
        init_search_env = &search_16_avx2_init;
        exit_search_env = &search_16_avx2_exit;
    }
    else if( is_sse2_enabled() ) {
        init_search_env = &search_16_sse2_init;
        exit_search_env = &search_16_sse2_exit;
    }
    else {
        ffatal( "\nAVX2 and SSE2 not enabled. No 16 bit search possible\n\n", search_type );
    }
}

unsigned long search_16_chunk( p_s16info s16info, p_minheap heap, p_db_chunk chunk, p_search_data sdp ) {
    unsigned long searches_done = 0;

    p_node overflow_list = 0;

    for( int q_id = 0; q_id < sdp->q_count; q_id++ ) {
        search_algo( s16info, chunk, heap, &overflow_list, q_id );

        searches_done += chunk->fill_pointer;
    }

    if( overflow_list ) {
        printf( "Overflow detected: re-aligning %ld sequences with 64 bit\n", ll_size( overflow_list ) ); // TODO show info or not?

        if( !s16info->hearray_64 ) {
            s16info->hearray_64 = search_63_init( sdp );
        }

        p_db_chunk overflow_chunk = convert_to_chunk( overflow_list );

        search_63_chunk( heap, overflow_chunk, sdp, s16info->hearray_64 );

        free( overflow_chunk->seq );
        free( overflow_chunk );
    }

    return searches_done;
}

void search_16( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo || !init_search_env || !exit_search_env ) {
        ffatal( "\n 16 bit search not initialized!!\n\n" );
    }

    p_s16info s16info = init_search_env( sdp );

    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        int searched_sequences = search_16_chunk( s16info, res->heap, chunk, sdp );

        assert( searched_sequences == chunk->fill_pointer * sdp->q_count );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    exit_search_env( s16info );
}
