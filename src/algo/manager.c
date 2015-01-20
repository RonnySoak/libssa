/*
 * manager.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "manager.h"

#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include <assert.h>

#include "../util/minheap.h"
#include "../util/thread_pool.h"
#include "../util/util.h"
#include "../libssa_datatypes.h"
#include "../db_iterator.h"
#include "aligner.h"
#include "searcher.h"

static int alignment_hit_count;

unsigned long max_chunk_size = 1000; // TODO change and/or make it configurable

static void init( p_query query, int hit_count, int search_type, int bit_width ) {
    alignment_hit_count = hit_count;

    s_init( search_type, bit_width, query, hit_count );

    a_init_data( search_type );

    it_init( max_chunk_size );
}

void init_for_sw( p_query query, int hit_count, int bit_width ) {
    init( query, hit_count, SMITH_WATERMAN,bit_width );
}

void init_for_nw( p_query query, int hit_count, int bit_width ) {
    init( query, hit_count, NEEDLEMAN_WUNSCH, bit_width );
}

void init_for_nw_sellers( p_query query, int hit_count, int bit_width ) {
    init( query, hit_count, NEEDLEMAN_WUNSCH_SELLERS, bit_width );
}

static int alignment_compare( const void * a, const void * b ) {
    alignment_p * x = (alignment_p *) a;
    alignment_p * y = (alignment_p *) b;

    int cmp = CMP_ASC( (*x)->score, (*y)->score );
    if( !cmp ) {
        cmp = CMP_ASC( (*x)->db_seq.ID, (*y)->db_seq.ID );
    }
    return cmp;
}

static void sort_alignment_list( p_alignment_list alist ) {
    qsort( alist->alignments, alist->len, sizeof(alignment_p), alignment_compare );
}

/**
 * Run a search for query in the database. Aligns the query sequence against
 * each sequence in the DB and returns 'hit_count' alignments. The search is
 * configured through set bits in 'flags'.
 */
p_alignment_list m_run() {
    init_thread_pool();

    start_threads( s_search );

    p_search_result search_result_list[_max_thread_count];

    unsigned long chunks_processed = 0;
    unsigned long db_sequences_processed = 0;

    wait_for_threads( (void **) &search_result_list );

    p_minheap search_results = minheap_init( alignment_hit_count );
    for( int i = 0; i < get_current_thread_count(); i++ ) {
        for( int j = 0; j < search_result_list[i]->heap->count; j++ ) {
            minheap_add( search_results, &search_result_list[i]->heap->array[j] );
        }

        chunks_processed += search_result_list[i]->chunk_count;
        db_sequences_processed += search_result_list[i]->seq_count;
    }

    assert( ssa_db_get_sequence_count() == db_sequences_processed );
    assert( ceil( ssa_db_get_sequence_count() / (double ) max_chunk_size ) == chunks_processed );

    minheap_sort( search_results );
    it_free();

    a_set_alignment_pairs( search_results->count, search_results->array );

    start_threads( a_align );

    p_alignment_list alist = xmalloc( alignment_hit_count * sizeof(struct alignment_list) );
    alist->alignments = xmalloc( alignment_hit_count * sizeof(alignment_t) );
    alist->len = alignment_hit_count;

    p_alignment_list align_result_list[get_current_thread_count()];

    wait_for_threads( (void **) &align_result_list );

    int alist_ptr = 0;
    for( int i = 0; i < get_current_thread_count(); i++ ) {
        for( int j = 0; j < align_result_list[i]->len; j++ ) {
            alist->alignments[alist_ptr++] = align_result_list[i]->alignments[j];
        }

        free( align_result_list[i]->alignments );
        free( align_result_list[i] );
    }

#if 0
    for( int i = 0; i < alist->len; i++ ) {
        for( int j = 0; j < alist->len; j++ ) {
            if( i != j ) {
                assert( alist->alignments[i]->db_seq.ID != alist->alignments[j]->db_seq.ID );
            }
            else {
                assert( alist->alignments[i]->db_seq.ID == alist->alignments[j]->db_seq.ID );
            }
        }
    }
#endif

    sort_alignment_list( alist );

    minheap_exit( search_results );

    a_free_data();
    for( int i = 0; i < _max_thread_count; i++ ) {
        s_free( search_result_list[i] );
    }

    return alist;
}
