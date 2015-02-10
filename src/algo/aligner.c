/*
 * aligner.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "aligner.h"

#include <stdlib.h>
#include <pthread.h>

#include "../matrices.h"
#include "../db_iterator.h"
#include "searcher.h"
#include "align.h"
#include "../util/util.h"
#include "../util/util_sequence.h"

static p_alignment_data adp = 0;
static size_t chunk_counter = 0;

static pthread_mutex_t chunk_mutex = PTHREAD_MUTEX_INITIALIZER;

void a_init_data( int search_type ) {
    adp = xmalloc( sizeof(struct alignment_data) );

    if( search_type == SMITH_WATERMAN ) {
        adp->align_function = &align_sw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        adp->align_function = &align_nw;
    }
    else if( search_type == NEEDLEMAN_WUNSCH_SELLERS ) {
        adp->align_function = &align_nw_sellers;
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

static void init_alignment( alignment_p a, elem_t * e, seq_buffer* queries ) {
    p_seqinfo info = it_get_sequence( e->db_id );
    if( !info ) {
        // TODO raise error, as this should not be possible
        ffatal( "Could not get sequence from DB: %ld", e->db_id );
    }

    /*
     * TODO find a better way, maybe move code for translating based on symtype,
     * etc to util_sequence.c
     */
    sequence dseq = us_prepare_sequence( info->seq, info->seqlen, e->dframe, e->dstrand );

    seq_buffer qseq = queries[e->query_id];

    a->db_seq.seq = dseq.seq;
    a->db_seq.len = dseq.len;
    a->db_seq.strand = e->dframe;
    a->db_seq.frame = e->dstrand;
    a->db_seq.header = info->header;
    a->db_seq.headerlen = info->headerlen;
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
}

void a_free( p_alignment_list alist ) {
    if( !alist ) {
        return;
    }

    if( alist->alignments ) {
        for( size_t i = 0; i < alist->len; i++ ) {
            if( alist->alignments[i] ) {
                alignment_p a = alist->alignments[i];

                free( a->db_seq.seq );
                a->db_seq.seq = 0;
                a->db_seq.len = 0;
                a->db_seq.strand = 0;
                a->db_seq.frame = 0;
                a->db_seq.header = 0;
                a->db_seq.headerlen = 0;
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

    p_alignment_list alignment_list = xmalloc( sizeof(struct alignment_list) );
    alignment_list->alignments = xmalloc( adp->pair_count * sizeof(alignment_t) );
    alignment_list->len = 0;

    elem_t * chunk = 0;
    while( (chunk = get_chunk()) != 0 ) {
        // do alignment for each pair
        alignment_p a = xmalloc( sizeof(alignment_t) );
        init_alignment( a, chunk, adp->queries );

        adp->align_function( a );

        alignment_list->alignments[alignment_list->len++] = a;
    }

    return alignment_list;
}
