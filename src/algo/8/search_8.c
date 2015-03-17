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

#include "search_8.h"
#include "search_8_util.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../searcher.h"
#include "../../util/util.h"
#include "../../util/linked_list.h"
#include "../../matrices.h"
#include "../../cpu_config.h"
#include "../../db_adapter.h"
#include "../16/search_16.h"

static void (*search_algo)( p_s8info, p_db_chunk, p_minheap, p_node *, uint8_t );

void search_8_init_algo( int search_type ) {
    if( !is_sse2_enabled() ) {
        ffatal( "\nAVX2 and SSE4.1 not enabled. No 8 bit search possible\n\n", search_type );
    }

    if( search_type == SMITH_WATERMAN ) {
        if( is_avx2_enabled() ) {
            search_algo = &search_8_avx2_sw;
        }
        else if( is_sse41_enabled() ) {
            search_algo = &search_8_sse41_sw;
        }
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        if( is_avx2_enabled() ) {
            search_algo = &search_8_avx2_nw;
        }
        else if( is_sse41_enabled() ) {
            search_algo = &search_8_sse41_nw;
        }
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }

}

p_s8info search_8_init( p_search_data sdp ) {
    if( is_avx2_enabled() ) {
        return search_8_avx2_init( sdp );
    }
    return search_8_sse41_init( sdp );
}

void search_8_exit( p_s8info s ) {
    /* free mem for dprofile, hearray, dir, qtable */
    if( s->hearray )
        free( s->hearray );
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

    if( s->s16info ) {
        search_16_exit( s->s16info );
        s->s16info = 0;
    }

    free( s );
}

static size_t search_8_chunk( p_s8info s8info, p_minheap heap, p_db_chunk chunk, p_search_data sdp ) {
    size_t overflown_seq_count = 0;

    p_node overflow_list = 0;

    for( uint8_t q_id = 0; q_id < sdp->q_count; q_id++ ) {
        search_algo( s8info, chunk, heap, &overflow_list, q_id );
    }

    if( overflow_list ) {
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
            s8info->s16info = search_16_init( sdp );
        }

        p_db_chunk overflow_chunk = convert_to_chunk( overflow_list );
        overflown_seq_count = overflow_chunk->fill_pointer;

        search_16_chunk( s8info->s16info, heap, overflow_chunk, sdp );

        free( overflow_chunk->seq );
        free( overflow_chunk );
    }

    return overflown_seq_count;
}

void search_8( p_db_chunk chunk, p_search_data sdp, p_search_result res ) {
    if( !search_algo ) {
        ffatal( "\n 8 bit search not initialized.\n" );
    }

    p_s8info s8info = search_8_init( sdp );

    adp_next_chunk( chunk );

    while( chunk->fill_pointer ) {
        size_t overflown_seq_count = search_8_chunk( s8info, res->heap, chunk, sdp );

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;
        res->overflow_8_bit_count += overflown_seq_count;

        adp_next_chunk( chunk );
    }

    search_8_exit( s8info );
}
