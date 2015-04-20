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

#include "search_16.h"
#include "search_16_util.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../searcher.h"
#include "../../util/minheap.h"
#include "../../util/util.h"
#include "../../matrices.h"
#include "../../cpu_config.h"
#include "../../db_adapter.h"
#include "../64/search_64.h"

static void (*search_algo)( p_s16info, p_db_chunk, p_minheap, p_db_chunk, uint8_t );

void search_16_init_algo( int search_type ) {
    if( !is_sse2_enabled() ) {
        fatal( "\nAVX2 and SSE2 not enabled. No 16 bit search possible\n\n", search_type );
    }

    if( search_type == SMITH_WATERMAN ) {
        if( is_avx2_enabled() ) {
            search_algo = &search_16_avx2_sw;
        }
        else {
            search_algo = &search_16_sse2_sw;
        }
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        if( is_avx2_enabled() ) {
            search_algo = &search_16_avx2_nw;
        }
        else {
            search_algo = &search_16_sse2_nw;
        }
    }
    else {
        fatal( "\nunknown search type: %d\n\n", search_type );
    }
}

p_s16info search_16_init( p_search_data sdp ) {
    if( is_avx2_enabled() ) {
        return search_16_avx2_init( sdp );
    }
    return search_16_sse2_init( sdp );
}

void search_16_exit( p_s16info s ) {
    if( s->hearray )
        free( s->hearray );
    if( s->hearray_64 )
        free( s->hearray_64 );
    if( s->dprofile )
        free( s->dprofile );

    for( int i = 0; i < s->q_count; i++ ) {
        if( s->queries[i]->q_table )
            free( s->queries[i]->q_table );

        s->queries[i]->q_len = 0;

        free( s->queries[i] );
        s->queries[i] = 0;
    }
    s->q_count = 0;

    free( s );
}

size_t search_16_chunk( p_s16info s16info, p_minheap heap, p_db_chunk chunk, p_search_data sdp ) {
    size_t overflown_seq_count = 0;

    p_db_chunk overflow_chunk = adp_alloc_chunk( chunk->size * sdp->q_count );

    for( uint8_t q_id = 0; q_id < sdp->q_count; q_id++ ) {
        search_algo( s16info, chunk, heap, overflow_chunk, q_id );
    }

    if( overflow_chunk->fill_pointer ) {
        if( !s16info->hearray_64 ) {
            s16info->hearray_64 = search_64_alloc_hearray( sdp );
        }

        overflown_seq_count = overflow_chunk->fill_pointer;

        search_64_chunk( heap, overflow_chunk, sdp, s16info->hearray_64 );
    }

    adp_free_chunk_no_sequences( overflow_chunk );

    return overflown_seq_count;
}

void search_16( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    assert( search_algo );

    p_s16info s16info = search_16_init( sdp );

    adp_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        size_t overflown_seq_count = search_16_chunk( s16info, res->heap, chunk, sdp );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;
        res->overflow_16_bit_count += overflown_seq_count;

        adp_next_chunk( chunk );
    }

    search_16_exit( s16info );
}
