/*
 * search_64.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "../64/search_64.h"

#include <stdlib.h>
#include <assert.h>

#include "../searcher.h"
#include "../../util/minheap.h"
#include "../../util/util.h"
#include "../../db_iterator.h"

static int64_t (*search_algo)( sequence_t*, sequence_t*, int64_t* );

void search_64_init_algo( int search_type ) {
    if( search_type == SMITH_WATERMAN ) {
        search_algo = &full_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        search_algo = &full_nw;
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }
}

void search_64_chunk( p_minheap heap, p_db_chunk chunk, p_search_data sdp, int64_t* hearray ) {
    for( uint8_t q_id = 0; q_id < sdp->q_count; q_id++ ) {
        seq_buffer_t query = sdp->queries[q_id];

        for( size_t i = 0; i < chunk->fill_pointer; i++ ) {
            p_sdb_sequence dseq = chunk->seq[i];

            long score = search_algo( &dseq->seq, &query.seq, hearray );

            add_to_minheap( heap, q_id, dseq, score );
        }
    }
}

int64_t* search_64_alloc_hearray( p_search_data sdp ) {
    return xmalloc( sdp->maxqlen * 32 );
}

void search_64( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo ) {
        ffatal( "\n 64 bit search not initialized!!\n\n" );
    }

    int64_t* hearray = search_64_alloc_hearray( sdp );

    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        search_64_chunk( res->heap, chunk, sdp, hearray );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    free( hearray );
}
