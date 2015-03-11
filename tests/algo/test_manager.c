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

#include "../tests.h"

#include "../../src/algo/manager.h"
#include "../../src/algo/aligner.h"
#include "../../src/algo/gap_costs.h"
#include "../../src/util/util.h"
#include "../../src/libssa.h"
#include "../../src/cpu_config.h"
#include "../../src/matrices.h"
#include "../../src/query.h"
#include "../../src/db_iterator.h"

static p_query setup_manager_test() {
    set_max_compute_capability( COMPUTE_ON_SSE41 );

    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
    mat_init_constant_scoring( 1, -1 );

    ssa_db_init( "./tests/testdata/test.fas" );
    gapO = 1;
    gapE = 1;

    return query_read_from_string( "query", "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
}

static void exit_manager_test( p_alignment_list alist ) {
    a_free( alist );
    mat_free();

    reset_compute_capability();
}

static void do_manager_test_std( p_query query, int align_type, size_t hit_count ) {
    p_alignment_list alist = m_run( hit_count );

    ck_assert_int_eq( 3, alist->len );

    for( int i = 0; i < alist->len; i++ ) {
        p_alignment a = alist->alignments[i];
        ck_assert_ptr_ne( NULL, a );

        ck_assert_ptr_eq( query->nt[0].seq, a->query.seq );

        // check for different database sequences
        for( int j = 0; j < alist->len; j++ ) {
            if( i != j ) {
                ck_assert_int_ne( a->db_seq.ID, alist->alignments[j]->db_seq.ID );
            }
        }

        if( align_type == COMPUTE_SCORE ) {
            ck_assert_ptr_eq( 0, a->alignment );
            ck_assert_int_eq( 0, a->alignment_len );
            ck_assert_int_eq( 0, a->align_d_end );
            ck_assert_int_eq( 0, a->align_d_start );
            ck_assert_int_eq( 0, a->align_q_end );
            ck_assert_int_eq( 0, a->align_q_start );
        }
        else {
            ck_assert_ptr_ne( 0, a->alignment );
            ck_assert_int_ne( 0, a->alignment_len );
            ck_assert_int_ne( 0, a->align_d_end );
            ck_assert_int_ne( 0, a->align_d_start );
            ck_assert_int_ne( 0, a->align_q_end );
            ck_assert_int_ne( 0, a->align_q_start );
        }
    }

    exit_manager_test( alist );
}

START_TEST (test_manager_simple_sw_64)
    {
        p_query query = setup_manager_test();

        init_for_sw( query, BIT_WIDTH_64, COMPUTE_ALIGNMENT );

        do_manager_test_std( query, COMPUTE_ALIGNMENT, 3 );
    }END_TEST

START_TEST (test_manager_simple_nw_64)
    {
        p_query query = setup_manager_test();

        init_for_sw( query, BIT_WIDTH_64, COMPUTE_SCORE );

        do_manager_test_std( query, COMPUTE_SCORE, 3 );
    }END_TEST

START_TEST (test_manager_simple_sw_16)
    {
        p_query query = setup_manager_test();

        init_for_sw( query, BIT_WIDTH_16, COMPUTE_SCORE );

        do_manager_test_std( query, COMPUTE_SCORE, 3 );
    }END_TEST

START_TEST (test_manager_simple_nw_16)
    {
        p_query query = setup_manager_test();

        init_for_sw( query, BIT_WIDTH_16, COMPUTE_ALIGNMENT );

        do_manager_test_std( query, COMPUTE_ALIGNMENT, 3 );
    }END_TEST

static void compare_test( void (*init_func)( p_query, int, int ), size_t hit_count ) {
    p_query query = setup_manager_test();

    init_func( query, BIT_WIDTH_64, COMPUTE_ALIGNMENT );
    p_alignment_list alist_64 = m_run( hit_count );

    it_reset_chunk_counter();

    init_func( query, BIT_WIDTH_16, COMPUTE_ALIGNMENT );
    p_alignment_list alist_16 = m_run( hit_count );

    ck_assert_int_eq( hit_count, alist_64->len );
    ck_assert_int_eq( hit_count, alist_16->len );

    for( size_t i = 0; i < hit_count; i++ ) {
        p_alignment a64 = alist_64->alignments[i];
        p_alignment a16 = alist_16->alignments[i];

        ck_assert_ptr_ne( NULL, a64 );
        ck_assert_ptr_ne( NULL, a16 );

        ck_assert_int_eq( a64->score, a16->score );
        ck_assert_int_eq( a64->db_seq.ID, a16->db_seq.ID );
        ck_assert_int_eq( a64->alignment_len, a16->alignment_len );
        ck_assert_str_eq( a64->alignment, a16->alignment );
        ck_assert_ptr_ne( 0, a64->alignment );
        ck_assert_ptr_ne( 0, a16->alignment );
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

        init_for_nw( query, BIT_WIDTH_64, COMPUTE_ALIGNMENT );
        p_alignment_list alist = m_run( 1 );

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
