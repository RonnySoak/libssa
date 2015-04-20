/*
 Copyright (C) 2014-2015 Jakob Frielingsdorf

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Contact: Jakob Frielingsdorf <jfrielingsdorf@gmail.com>
 */

#include <stdlib.h>
#include <assert.h>

#include "../db_adapter.h"
#include "../util/util.h"
#include "../util/minheap.h"
#include "../matrices.h"
#include "../util/util_sequence.h"

#include "16/search_16.h"
#include "64/search_64.h"
#include "8/search_8.h"

static p_search_data sdp = 0;
static void (*search_func)( p_db_chunk, p_search_data, p_search_result );

static void add_to_buffer( seq_buffer_t* buf, sequence_t seq, int strand, int frame ) {
    buf->seq = seq;
    buf->frame = frame;
    buf->strand = strand;
}

p_search_data s_create_searchdata( p_query query ) {
    p_search_data sdp = xmalloc( sizeof(search_data_t) );

    size_t qlen = 0;
    size_t maxqlen = 0;

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

                maxqlen = qlen > maxqlen ? qlen : maxqlen;
            }
    }
    else if( (symtype == AMINOACID) || (symtype == TRANS_DB) ) {
        qlen = query->aa[0].len;

        add_to_buffer( &sdp->queries[sdp->q_count++], query->aa[0], 0, 0 );

        maxqlen = qlen > maxqlen ? qlen : maxqlen;
    }
    else if( (symtype == TRANS_QUERY) || (symtype == TRANS_BOTH) ) {
        for( int s = 0; s < 2; s++ )
            if( (s + 1) & query_strands )
                for( int f = 0; f < 3; f++ ) {
                    qlen = query->aa[3 * s + f].len;

                    add_to_buffer( &sdp->queries[sdp->q_count++], query->aa[3 * s + f], s, f );

                    maxqlen = qlen > maxqlen ? qlen : maxqlen;
                }
    }

    sdp->maxqlen = maxqlen;

    return sdp;
}

int s_get_query_count() {
    return sdp->q_count;
}

seq_buffer_t s_get_query( int idx ) {
    return sdp->queries[idx];
}

void s_init( int search_type, int bit_width, p_query query ) {
    /*
     * Here we initialize all algorithms, to use them as fallbacks if one overflows.
     *
     * Although all are initialized, the main algorithm is the one specified.
     */
    if( bit_width == BIT_WIDTH_64 ) {
        search_func = &search_64;
    }
    else if( bit_width == BIT_WIDTH_16 ) {
        search_func = &search_16;
    }
    else if( bit_width == BIT_WIDTH_8 ) {
        search_func = &search_8;
    }
    else {
        fatal( "\nunknown bit width provided: %d\n\n", bit_width );
    }

    search_64_init_algo( search_type );
    search_16_init_algo( search_type );
    search_8_init_algo( search_type );

    sdp = s_create_searchdata( query );
}

static void s_free_search_data( p_search_data sdp ) {
    if( sdp ) {
        sdp->q_count = 0;

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

/*
 * Performs a database search.
 *
 * @param hit_count (type: size_t) number of expected results
 */
void * s_search( void * hit_count ) {
    assert( search_func );
    assert( sdp );
    assert( hit_count );

    p_search_result res = xmalloc( sizeof(search_result_t) );
    res->heap = minheap_init( *((size_t *) hit_count) );
    res->chunk_count = 0;
    res->seq_count = 0;
    res->overflow_8_bit_count = 0;
    res->overflow_16_bit_count = 0;

    p_db_chunk chunk = adp_init_new_chunk();

    search_func( chunk, sdp, res );

    adp_free_chunk( chunk );

    return res;
}
