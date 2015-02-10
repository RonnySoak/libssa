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
#include "../query.h"

#include "63/search_63.h"
#include "16/search_16.h"
#include "8/search_8.h"

static p_search_data sdp = 0;
static void (*search_func)( p_db_chunk, p_search_data, p_search_result );

static void add_to_buffer( seq_buffer* buf, sequence seq, int strand, int frame ) {
    buf->seq = seq;
    buf->frame = frame;
    buf->strand = strand;
}

p_search_data s_create_searchdata( p_query query, size_t hit_count ) {
    p_search_data sdp = xmalloc( sizeof(struct search_data) );

    sdp->hit_count = hit_count;

    unsigned long qlen = 0;
    unsigned long hearraylen = 0;

    sdp->q_count = 0;

    // init buffer
    for( int i = 0; i < 6; i++ ) {
        sdp->queries[i].seq.len = 0;
        sdp->queries[i].seq.seq = 0;
        sdp->queries[i].strand = 0;
        sdp->queries[i].frame = 0;
    }

    if( symtype == NUCLEOTIDE ) {
        for( int s = 0; s < 2; s++ )
            if( (s + 1) & query_strands ) {
                qlen = query->nt[s].len;

                add_to_buffer( &sdp->queries[sdp->q_count++], query->nt[s], s, 0 );

                hearraylen = qlen > hearraylen ? qlen : hearraylen;
            }
    }
    else if( (symtype == AMINOACID) || (symtype == TRANS_DB) ) {
        qlen = query->aa[0].len;

        add_to_buffer( &sdp->queries[sdp->q_count++], query->aa[0], 0, 0 );

        hearraylen = qlen > hearraylen ? qlen : hearraylen;
    }
    else if( (symtype == TRANS_QUERY) || (symtype == TRANS_BOTH) ) {
        for( int s = 0; s < 2; s++ )
            if( (s + 1) & query_strands )
                for( int f = 0; f < 3; f++ ) {
                    qlen = query->aa[3 * s + f].len;

                    add_to_buffer( &sdp->queries[sdp->q_count++], query->aa[3 * s + f], s, f );

                    hearraylen = qlen > hearraylen ? qlen : hearraylen;
                }
    }

    sdp->hearraylen = hearraylen;

    return sdp;
}

int s_get_query_count() {
    return sdp->q_count;
}

seq_buffer s_get_query( int idx ) {
    return sdp->queries[idx];
}

void s_init( int search_type, int bit_width, p_query query, size_t hit_count ) {
    /*
     * Here we initialize all algorithms, to use them as fallbacks if one overflows.
     *
     * Although all are initialized, the main algorithm is the one specified.
     */
    if( bit_width == BIT_WIDTH_64 ) {
        search_func = &search_63;
    }
    else if( bit_width == BIT_WIDTH_16 ) {
        search_func = &search_16;
    }
    else if( bit_width == BIT_WIDTH_8 ) {
        search_func = &search_8;
    }
    else {
        ffatal( "\nunknown bit width provided: %d\n\n", bit_width );
    }

    search63_init_algo( search_type );
    search_16_init_algo( search_type );
    search_8_init_algo( search_type );

    sdp = s_create_searchdata( query, hit_count );
}

void s_free_search_data( p_search_data sdp ) {
    if( sdp ) {
        sdp->q_count = 0;
        sdp->hit_count = 0;

        free( sdp );
    }
}

void s_free( p_search_result res ) {
    if( !res ) {
        return;
    }

    s_free_search_data( sdp );
    sdp = 0;

    minheap_exit( res->heap );

    res->chunk_count = 0;
    res->seq_count = 0;
    free( res );
    res = 0;
}

void * s_search( void * not_used ) {
    if( !search_func || !sdp ) {
        ffatal( "\n Search module not initialized!!\n\n" );
    }

    p_search_result res = xmalloc( sizeof(struct search_result) );
    res->heap = minheap_init( sdp->hit_count );
    res->chunk_count = 0;
    res->seq_count = 0;

    p_db_chunk chunk = it_init_new_chunk();

    search_func( chunk, sdp, res );

    it_free_chunk( chunk );

    return res;
}
