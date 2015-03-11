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

#include "aligner.h"

#include <stdlib.h>
#include <pthread.h>

#include "../matrices.h"
#include "../libssa_extern_db.h"
#include "searcher.h"
#include "align.h"
#include "../util/util.h"
#include "../util/util_sequence.h"

static p_alignment_data adp = 0;
static size_t chunk_counter = 0;

static pthread_mutex_t chunk_mutex = PTHREAD_MUTEX_INITIALIZER;

void a_init_data( int search_type ) {
    adp = xmalloc( sizeof(alignment_data_t) );

    if( search_type == SMITH_WATERMAN ) {
        adp->align_function = &align_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        adp->align_function = &align_nw;
    }
    else {
        ffatal( "\nunknown search type: %d\n\n", search_type );
    }

    adp->q_count = s_get_query_count();
    for( int i = 0; i < s_get_query_count(); i++ ) {
        adp->queries[i] = s_get_query( i );
    }
}

void a_free_data() {
    if( !adp ) {
        return;
    }

    adp->q_count = 0;
    adp->align_function = 0;
    adp->result_sequence_pairs = 0;

    chunk_counter = 0;

    free( adp );
}

static p_alignment init_alignment( elem_t * e, seq_buffer_t* queries ) {
    p_alignment a = xmalloc( sizeof(alignment_t) );

    p_seqinfo info = ssa_db_get_sequence( e->db_id );
    if( !info ) {
        // TODO raise error, as this should not be possible
        ffatal( "Could not get sequence from DB: %ld", e->db_id );
    }

    sequence_t dseq = us_prepare_sequence( info->seq, info->seqlen, e->dframe, e->dstrand );

    seq_buffer_t qseq = queries[e->query_id];

    a->db_seq.seq = dseq.seq;
    a->db_seq.len = dseq.len;
    a->db_seq.strand = e->dframe;
    a->db_seq.frame = e->dstrand;
    a->db_seq.ID = info->ID;

    a->query.seq = qseq.seq.seq;
    a->query.len = qseq.seq.len;
    a->query.strand = qseq.strand;
    a->query.frame = qseq.frame;

    a->align_q_start = 0;
    a->align_d_start = 0;
    a->align_q_end = 0;
    a->align_d_end = 0;
    a->alignment = 0;
    a->score = e->score;

    return a;
}

void create_score_alignment_list( p_minheap search_results, p_alignment_list alist ) {
    for( int i = 0; i < search_results->count; ++i ) {
        alist->alignments[i] = init_alignment( &search_results->array[i], adp->queries );
    }
}

void a_free( p_alignment_list alist ) {
    if( !alist ) {
        return;
    }

    if( alist->alignments ) {
        for( size_t i = 0; i < alist->len; i++ ) {
            if( alist->alignments[i] ) {
                p_alignment a = alist->alignments[i];

                free( a->db_seq.seq );
                a->db_seq.seq = 0;
                a->db_seq.len = 0;
                a->db_seq.strand = 0;
                a->db_seq.frame = 0;
                a->db_seq.ID = 0;

                a->query.seq = 0;
                a->query.len = 0;
                a->query.strand = 0;
                a->query.frame = 0;

                a->align_q_start = 0;
                a->align_d_start = 0;
                a->align_q_end = 0;
                a->align_d_end = 0;
                if( a->alignment ) {
                    free( a->alignment );
                    a->alignment = 0;
                }
                a->score = 0;

                free( a );
            }
            alist->alignments[i] = 0;
        }
        free( alist->alignments );
    }
    alist->alignments = 0;
    alist->len = 0;

    free( alist );
}

void a_set_alignment_pairs( size_t pair_count, elem_t * result_sequence_pairs ) {
    adp->pair_count = pair_count;
    adp->result_sequence_pairs = result_sequence_pairs;
}

static elem_t * get_chunk() {
    size_t next_chunk;

    pthread_mutex_lock( &chunk_mutex );
    next_chunk = chunk_counter++;
    pthread_mutex_unlock( &chunk_mutex );

    if( next_chunk >= adp->pair_count ) {
        return 0;
    }
    return &adp->result_sequence_pairs[next_chunk];
}

void * a_align( void * unused ) {
    if( !adp ) {
        ffatal( "\n Alignment module not initialized!!\n\n" );
    }

    p_alignment_list alignment_list = xmalloc( sizeof(alignment_list_t) );
    alignment_list->alignments = xmalloc( adp->pair_count * sizeof(alignment_t) );
    alignment_list->len = 0;

    elem_t * chunk = 0;
    while( (chunk = get_chunk()) != 0 ) {
        // do alignment for each pair
        p_alignment a = init_alignment( chunk, adp->queries );

        adp->align_function( a );

        alignment_list->alignments[alignment_list->len++] = a;
    }

    return alignment_list;
}
