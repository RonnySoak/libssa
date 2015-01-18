/*
 * search_16.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "search_16.h"
#include <stdlib.h>
#include <assert.h>

#include "../searcher.h"
#include "../../util/minheap.h"
#include "../../util/util.h"
#include "../../db_iterator.h"

static void (*search_algo)( p_s16info, p_db_chunk, p_minheap, int );

void init_algo_16( int search_type ) {
    if( search_type == SMITH_WATERMAN ) {
        search_algo = &search16_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        search_algo = &search16_nw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH_SELLERS ) {
//        search_algo = &search16_nw_sellers; TODO not yet implemented
        printf( "\nnot yet implemented\n\n" );
    }
}

static unsigned long search_chunk( p_s16info s16info, p_minheap heap, p_db_chunk chunk, p_search_data sdp ) {
    unsigned long searches_done = 0;

    for( int q_id = 0; q_id < sdp->q_count; q_id++ ) {
        seq_buffer query = sdp->queries[q_id];

        search_algo( s16info, chunk, heap, q_id );

        searches_done += chunk->fill_pointer;
    }

    return searches_done;
}

void search_16( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    p_s16info s16info = search16_init( gapO, gapE );

    // TODO do this for all translations of the query at once, and store all in the s16info struct
//    search16_init_query( s16info, query->nt[0].seq, query->nt[0].len );

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
