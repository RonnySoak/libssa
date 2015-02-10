/*
 * search_63.c
 *
 *  Created on: Jan 17, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "search_63.h"

#include <stdlib.h>
#include <assert.h>

#include "../searcher.h"
#include "../../util/minheap.h"
#include "../../util/util.h"
#include "../../db_iterator.h"

static int64_t (*search_algo)( sequence*, sequence*, int64_t* );

void search63_init_algo( int search_type ) {
    if( search_type == SMITH_WATERMAN ) {
        search_algo = &full_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        search_algo = &full_nw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH_SELLERS ) {
        search_algo = &full_nw_sellers;
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }
}

unsigned long search_63_chunk( p_minheap heap, p_db_chunk chunk, p_search_data sdp, int64_t* hearray ) {
    unsigned long searches_done = 0;

    for( int q_id = 0; q_id < sdp->q_count; q_id++ ) {
        seq_buffer query = sdp->queries[q_id];

        for( size_t i = 0; i < chunk->fill_pointer; i++ ) {
            p_sdb_sequence dseq = chunk->seq[i];

            long score = search_algo( &dseq->seq, &query.seq, hearray );

            add_to_minheap( heap, q_id, dseq, score );

            searches_done++;
        }
    }

    return searches_done;
}

int64_t* search_63_init_hearray( p_search_data sdp ) {
    return xmalloc( sdp->hearraylen * 32 );
}

void search_63( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo ) {
        ffatal( "\n 64 bit search not initialized!!\n\n" );
    }

    int64_t* hearray = search_63_init_hearray( sdp );

    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        int searched_sequences = search_63_chunk( res->heap, chunk, sdp, hearray );

        assert( searched_sequences == chunk->fill_pointer * sdp->q_count );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    free( hearray );
}
