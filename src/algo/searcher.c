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

#include "63/search_63.h"
#include "16/search_16.h"

void s_init( int search_type, int bit_width ) {
    if( bit_width == 64 ) {
        init_algo_63( search_type );
    }
    else if( bit_width == 16 ) {
        init_algo_16( search_type );
    }
    else {
        printf( "\nnot implemented yet!\n\n" );
    }
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

    p_db_chunk chunk = it_new_chunk();

    search_63( chunk, sdp, res );

    it_free_chunk( chunk );

    return res;
}
