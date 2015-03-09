/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/algo/aligner.h"
#include "../../src/algo/align.h"
#include "../../src/algo/manager.h"

#include "../../src/util/util.h"
#include "../../src/util/util_sequence.h"
#include "../../src/libssa.h"
#include "../../src/util/minheap.h"
#include "../../src/matrices.h"
#include "../../src/db_iterator.h"
#include "../../src/query.h"
#include "../../src/algo/searcher.h"
#include "../../src/algo/gap_costs.h"

static elem_t new_elem( int ID, int frame, int strand, int qid, long score ) {
    elem_t e;
    e.db_id = ID;
    e.dframe = frame;
    e.dstrand = strand;
    e.query_id = qid;
    e.score = score;

    return e;
}

static p_query setup_aligner_test( char * query_string, char * db_file, int chunk_size ) {
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
    mat_init_constant_scoring( 1, -1 );

    p_query query = query_read_from_string( "short query", query_string );
    ssa_db_init( concat( "./tests/testdata/", db_file ) );

    it_init( chunk_size );
    gapO = 1;
    gapE = 1;

    return query;
}

static p_alignment_list do_aligner_test_step_two( int search_type, p_query query, int hit_count, int pair_count, elem_t * result_sequence_pairs) {
    s_init( search_type, BIT_WIDTH_64, query, hit_count );

    a_init_data( search_type );

    a_set_alignment_pairs( pair_count, result_sequence_pairs );

    p_alignment_list alist = a_align( NULL );

    ck_assert_int_eq( hit_count, alist->len );

    return alist;
}

static void exit_aligner_test( p_alignment_list alist, p_query query ) {
    a_free( alist );

    mat_free();
    query_free( query );
    it_exit();
    ssa_db_close();
}

START_TEST (test_aligner_simple_sw)
    {
        p_query query = setup_aligner_test( "AT", "short_db.fas", 1 );

        sequence_t sdb = us_prepare_sequence( "AATG", 4, 0, 0 );

        elem_t e1 = new_elem( 0, 0, 0, 0, 2 );
        elem_t * elements = { &e1 };

        p_alignment_list alist = do_aligner_test_step_two( SMITH_WATERMAN, query, 1, 1, elements );

        p_alignment al = alist->alignments[0];

        ck_assert_str_eq( sdb.seq, al->db_seq.seq );
        ck_assert_int_eq( sdb.len, al->db_seq.len );
        ck_assert_int_eq( 0, al->db_seq.frame );
        ck_assert_int_eq( 0, al->db_seq.strand );
        ck_assert_int_eq( 0, al->db_seq.ID );

        ck_assert_str_eq( query->nt[0].seq, al->query.seq );
        ck_assert_int_eq( query->nt[0].len, al->query.len );
        ck_assert_int_eq( 0, al->query.frame );
        ck_assert_int_eq( 0, al->query.strand );

        ck_assert_int_eq( 2, al->score ); // the score is 2, since we set it to 2
        ck_assert_int_eq( 1, al->align_d_start );
        ck_assert_int_eq( 2, al->align_d_end );
        ck_assert_int_eq( 0, al->align_q_start );
        ck_assert_int_eq( 1, al->align_q_end );

        ck_assert_str_eq( "2M", al->alignment );

        exit_aligner_test( alist, query );
    }END_TEST

START_TEST (test_aligner_simple_sw_2)
    {
        p_query query = setup_aligner_test( "ATGCAAAAA", "one_seq_db.fas", 1 );

        elem_t e1 = new_elem( 0, 0, 0, 0, 9 );
        elem_t * elements = { &e1 };

        p_alignment_list alist = do_aligner_test_step_two( SMITH_WATERMAN, query, 1, 1, elements );

        p_alignment al = alist->alignments[0];

        ck_assert_str_eq( "9M", al->alignment );

        exit_aligner_test( alist, query );
    }END_TEST

START_TEST (test_aligner_more_sequences_sw)
    {
        p_query query = setup_aligner_test( "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA", "test.fas", 3 );

        p_minheap heap = minheap_init( 5 );

        p_seqinfo seqInfo = ssa_db_get_sequence( 0 );
        sequence_t sdb0 = us_prepare_sequence( seqInfo->seq, seqInfo->seqlen, 0, 0 );
        elem_t e = new_elem( 0, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        e = new_elem( 1, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        e = new_elem( 2, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        e = new_elem( 3, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        e = new_elem( 4, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        p_alignment_list alist = do_aligner_test_step_two( SMITH_WATERMAN, query, 5, heap->count, heap->array );
        minheap_exit( heap );

        p_alignment al = alist->alignments[0];

        ck_assert_str_eq( sdb0.seq, al->db_seq.seq );
        ck_assert_int_eq( sdb0.len, al->db_seq.len );
        ck_assert_int_eq( 0, al->db_seq.frame );
        ck_assert_int_eq( 0, al->db_seq.strand );
        ck_assert_int_eq( 0, al->db_seq.ID );

        ck_assert_str_eq( query->nt[0].seq, al->query.seq );
        ck_assert_int_eq( query->nt[0].len, al->query.len );
        ck_assert_int_eq( 0, al->query.frame );
        ck_assert_int_eq( 0, al->query.strand );

        ck_assert_int_eq( 2, al->score ); // the score is 2, since we set it to 2
        ck_assert_int_eq( 31, al->align_d_start );
        ck_assert_int_eq( 54, al->align_d_end );
        ck_assert_int_eq( 10, al->align_q_start );
        ck_assert_int_eq( 34, al->align_q_end );
        ck_assert_str_eq( "20MD4M", al->alignment );


        exit_aligner_test( alist, query );
    }END_TEST

START_TEST (test_aligner_simple_nw)
    {
        p_query query = setup_aligner_test( "AT", "short_db.fas", 1 );

        sequence_t sdb = us_prepare_sequence( "AATG", 4, 0, 0 );

        elem_t e1 = new_elem( 0, 0, 0, 0, 2 );
        elem_t * elements = { &e1 };

        p_alignment_list alist = do_aligner_test_step_two( NEEDLEMAN_WUNSCH, query, 1, 1, elements );

        p_alignment al = alist->alignments[0];

        ck_assert_str_eq( sdb.seq, al->db_seq.seq );
        ck_assert_int_eq( sdb.len, al->db_seq.len );
        ck_assert_int_eq( 0, al->db_seq.frame );
        ck_assert_int_eq( 0, al->db_seq.strand );
        ck_assert_int_eq( 0, al->db_seq.ID );

        ck_assert_str_eq( query->nt[0].seq, al->query.seq );
        ck_assert_int_eq( query->nt[0].len, al->query.len );
        ck_assert_int_eq( 0, al->query.frame );
        ck_assert_int_eq( 0, al->query.strand );

        ck_assert_int_eq( 2, al->score ); // the score is 2, since we set it to 2
        ck_assert_int_eq( 0, al->align_d_start );
        ck_assert_int_eq( 3, al->align_d_end );
        ck_assert_int_eq( 0, al->align_q_start );
        ck_assert_int_eq( 1, al->align_q_end );

        ck_assert_str_eq( "MIMI", al->alignment );

        exit_aligner_test( alist, query );
    }END_TEST

void addAlignerTC( Suite *s ) {
    TCase *tc_core = tcase_create( "aligner" );
    tcase_add_test( tc_core, test_aligner_simple_sw );
    tcase_add_test( tc_core, test_aligner_simple_sw_2 );
    tcase_add_test( tc_core, test_aligner_simple_nw );
    tcase_add_test( tc_core, test_aligner_more_sequences_sw );

    suite_add_tcase( s, tc_core );
}
