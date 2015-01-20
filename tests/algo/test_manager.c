/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/algo/manager.h"
#include "../../src/algo/aligner.h"
#include "../../src/algo/search.h"

#include "../../src/util/util.h"
#include "../../src/libssa.h"
#include "../../src/matrices.h"
#include "../../src/query.h"
#include "../../src/db_iterator.h"

static p_query setup_manager_test() {
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
    mat_init_constant_scoring( 1, -1 );

    ssa_db_init_fasta( "./tests/testdata/test.fas" );
    gapO = 1;
    gapE = 1;

    return query_read_from_string( "query", "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
}

static void exit_manager_test( p_alignment_list alist ) {
    a_free( alist );

    mat_free();
}

static void do_manager_test_std( p_query query ) {
    p_alignment_list alist = m_run();

    ck_assert_int_eq( 3, alist->len );

    for( int i = 0; i < alist->len; i++ ) {
        alignment_p a = alist->alignments[i];
        ck_assert_ptr_ne( NULL, a );

        ck_assert_ptr_eq( query->nt[0].seq, a->query.seq );

        // check for different database sequences
        for( int j = 0; j < alist->len; j++ ) {
            if( i != j ) {
                ck_assert_int_ne( a->db_seq.header, alist->alignments[j]->db_seq.header );
                ck_assert_int_ne( a->db_seq.ID, alist->alignments[j]->db_seq.ID );
            }
        }
    }

    exit_manager_test( alist );
}

START_TEST (test_manager_simple_sw_64)
    {
        p_query query = setup_manager_test();

        init_for_sw( query, 3, BIT_WIDTH_64 );

        do_manager_test_std( query );
    }END_TEST

START_TEST (test_manager_simple_nw_64)
    {
        p_query query = setup_manager_test();

        init_for_sw( query, 3, BIT_WIDTH_64 );

        do_manager_test_std( query );
    }END_TEST

START_TEST (test_manager_simple_sw_16)
    {
        p_query query = setup_manager_test();

        init_for_sw( query, 3, BIT_WIDTH_16 );

        do_manager_test_std( query );
    }END_TEST

START_TEST (test_manager_simple_nw_16)
    {
        p_query query = setup_manager_test();

        init_for_sw( query, 3, BIT_WIDTH_16 );

        do_manager_test_std( query );
    }END_TEST

static void compare_test( void (*init_func)( p_query, int, int ), int hit_count ) {
    p_query query = setup_manager_test();

    init_func( query, hit_count, BIT_WIDTH_64 );
    p_alignment_list alist_64 = m_run();

    ssa_db_reset_sequence_counter();
    it_reset_chunk_counter();

    init_func( query, hit_count, BIT_WIDTH_16 );
    p_alignment_list alist_16 = m_run();

    ck_assert_int_eq( hit_count, alist_64->len );
    ck_assert_int_eq( hit_count, alist_16->len );

    for( int i = 0; i < hit_count; i++ ) {
        alignment_p a64 = alist_64->alignments[i];
        alignment_p a16 = alist_16->alignments[i];

        ck_assert_ptr_ne( NULL, a64 );
        ck_assert_ptr_ne( NULL, a16 );

        ck_assert_int_eq( a64->score, a16->score );
        ck_assert_int_eq( a64->db_seq.ID, a16->db_seq.ID );
        ck_assert_int_eq( a64->alignment_len, a16->alignment_len );
        ck_assert_str_eq( a64->alignment, a16->alignment );
    }

    exit_manager_test( alist_64 );
}

START_TEST (test_manager_compare_results_nw)
    {
        compare_test( &init_for_nw, 5 );
    }END_TEST

START_TEST (test_manager_compare_results_sw)
    {
        compare_test( &init_for_sw, 5 );
    }END_TEST

START_TEST (test_manager_one_hit)
    {
        p_query query = setup_manager_test();

        init_for_nw( query, 1, BIT_WIDTH_64 );
        p_alignment_list alist = m_run();

        ck_assert_int_eq( 1, alist->len );

        ck_assert_ptr_ne( NULL, alist->alignments[0] );
        ck_assert_ptr_eq( query->nt[0].seq, alist->alignments[0]->query.seq );

        exit_manager_test( alist );
    }END_TEST

void addManagerTC( Suite *s ) {
    TCase *tc_core = tcase_create( "manager" );
    tcase_add_test( tc_core, test_manager_simple_sw_64 );
    tcase_add_test( tc_core, test_manager_simple_nw_64 );
    tcase_add_test( tc_core, test_manager_simple_sw_16 );
    tcase_add_test( tc_core, test_manager_simple_nw_16 );
    tcase_add_test( tc_core, test_manager_compare_results_nw );
    tcase_add_test( tc_core, test_manager_compare_results_sw );
    tcase_add_test( tc_core, test_manager_one_hit );

    suite_add_tcase( s, tc_core );
}
