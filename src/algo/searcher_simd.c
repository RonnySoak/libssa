/*
 * searcher.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "search.h"

#include <stdlib.h>
#include <assert.h>

#include "../util/util.h"
#include "../util/minheap.h"
#include "../db_iterator.h"
#include "../matrices.h"
#include "align_simd.h"

static unsigned long search_chunk( p_minheap heap, p_db_chunk chunk, struct s16info * s ) {
    unsigned long searches_done = 0;

    for( int q_id = 0; q_id < sdp->q_count; q_id++ ) {
        seq_buffer query = sdp->queries[q_id];

        search16_init_query( s, query.seq.seq, query.seq.len );

        search16( s, chunk, heap, q_id );

        searches_done += chunk->fill_pointer;
    }

    return searches_done;
}

void s_free( p_search_result res ) {
    if( !res ) {
        return;
    }

    minheap_exit( res->heap );

    res->chunk_count = 0;
    res->seq_count = 0;
    free( res );
    res = 0;
}

void * s_search( void * search_data ) {
    p_search_data sdp = (p_search_data) search_data;

    p_search_result res = xmalloc( sizeof(struct search_result) );
    res->heap = minheap_init( sdp->hit_count );
    res->chunk_count = 0;
    res->seq_count = 0;

    p_s16info s = search16_init( gapO, gapE );

    p_db_chunk chunk = it_new_chunk();
    it_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        int searched_sequences = search_chunk( res->heap, chunk, s );

        assert( searched_sequences == chunk->fill_pointer * sdp->q_count );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk( chunk );
    }

    it_free_chunk( chunk );
    search16_exit( s );

    return res;
}
