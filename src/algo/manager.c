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

static size_t alignment_hit_count = 0;
static int align_type;

size_t max_chunk_size = 1000; // TODO change and/or make it configurable

static void init( p_query query, size_t hit_count, int search_type, int bit_width, int al_type ) {
    alignment_hit_count = hit_count;
    align_type = al_type;

    s_init( search_type, bit_width, query, hit_count );

    a_init_data( search_type );

    it_init( max_chunk_size );

#ifdef DBG_COLLECT_ALIGNED_DB_SEQUENCES
    char * desc = xmalloc( 14 );
    sprintf( desc,"%d_bit_type_%d", bit_width, search_type );

    // TODO works only for non translated sequences
    dbg_init_aligned_sequence_collecting( desc, ssa_db_get_sequence_count() );
#endif
}

void init_for_sw( p_query query, size_t hit_count, int bit_width, int align_type ) {
    init( query, hit_count, SMITH_WATERMAN, bit_width, align_type );
}

void init_for_nw( p_query query, size_t hit_count, int bit_width, int align_type ) {
    init( query, hit_count, NEEDLEMAN_WUNSCH, bit_width, align_type );
}

void init_for_nw_sellers( p_query query, size_t hit_count, int bit_width, int align_type ) {
    init( query, hit_count, NEEDLEMAN_WUNSCH_SELLERS, bit_width, align_type );
}

static int alignment_compare( const void * a, const void * b ) {
    p_alignment * x = (p_alignment *) a;
    p_alignment * y = (p_alignment *) b;

    int cmp = CMP_ASC( (*x)->score, (*y)->score );
    if( !cmp ) {
        cmp = CMP_ASC( (*x)->db_seq.ID, (*y)->db_seq.ID );
    }
    return cmp;
}

static void sort_alignment_list( p_alignment_list alist ) {
    qsort( alist->alignments, alist->len, sizeof(p_alignment), alignment_compare );
}

static p_alignment_list do_align( p_minheap search_results ) {
    p_alignment_list alist = xmalloc( alignment_hit_count * sizeof(struct alignment_list) );
    alist->alignments = xmalloc( alignment_hit_count * sizeof(struct alignment) );
    alist->len = alignment_hit_count;

    if( align_type == COMPUTE_SCORE ) {
        create_score_alignment_list( search_results, alist );
    }
    else {
        a_set_alignment_pairs( search_results->count, search_results->array );

        start_threads( a_align );

        p_alignment_list align_result_list[get_current_thread_count()];

        wait_for_threads( (void **) &align_result_list );

        int alist_ptr = 0;
        for( size_t i = 0; i < get_current_thread_count(); i++ ) {
            for( size_t j = 0; j < align_result_list[i]->len; j++ ) {
                alist->alignments[alist_ptr++] = align_result_list[i]->alignments[j];
            }

            free( align_result_list[i]->alignments );
            free( align_result_list[i] );
        }
    }
    sort_alignment_list( alist );

    return alist;
}

/**
 * Run a search for query in the database. Aligns the query sequence against
 * each sequence in the DB and returns 'hit_count' alignments. The search is
 * configured through set bits in 'flags'.
 */
p_alignment_list m_run() {
    if( !alignment_hit_count ) {
        ffatal( "\n Manager module not initialized!!\n\n" );
    }

    init_thread_pool();

    start_threads( s_search );

    p_search_result search_result_list[max_thread_count];

    unsigned long chunks_processed = 0;
    unsigned long db_sequences_processed = 0;

    wait_for_threads( (void **) &search_result_list );

#ifdef DBG_COLLECT_ALIGNED_DB_SEQUENCES
    dbg_print_aligned_sequences();
#endif

    size_t overflow_8_bit_count = 0;
    size_t overflow_16_bit_count = 0;

    p_minheap search_results = minheap_init( alignment_hit_count );
    for( size_t i = 0; i < get_current_thread_count(); i++ ) {
        for( size_t j = 0; j < search_result_list[i]->heap->count; j++ ) {
            minheap_add( search_results, &search_result_list[i]->heap->array[j] );
        }

        overflow_8_bit_count += search_result_list[i]->overflow_8_bit_count;
        overflow_16_bit_count += search_result_list[i]->overflow_16_bit_count;

        chunks_processed += search_result_list[i]->chunk_count;
        db_sequences_processed += search_result_list[i]->seq_count;
    }

    if( overflow_8_bit_count || overflow_16_bit_count ) {
        outf( "Overflow occurred: %ld sequences were re-aligned with 16 bit, and %ld sequences with 64 bit\n",
                overflow_8_bit_count, overflow_16_bit_count );
    }

    assert( ssa_db_get_sequence_count() == db_sequences_processed );
    assert( ceil( ssa_db_get_sequence_count() / (double ) max_chunk_size ) == chunks_processed );

    minheap_sort( search_results );
    it_exit();

    p_alignment_list alist = do_align( search_results );

    minheap_exit( search_results );

    a_free_data();
    for( size_t i = 0; i < max_thread_count; i++ ) {
        s_free( search_result_list[i] );
    }

    return alist;
}
