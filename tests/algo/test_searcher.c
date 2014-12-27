/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/util/util.h"
#include "../../src/libssa.h"
#include "../../src/util/minheap.h"
#include "../../src/matrices.h"
#include "../../src/db_iterator.h"
#include "../../src/query.h"
#include "../../src/algo/searcher.h"
#include "../../src/algo/search.h"
#include "../../src/algo/manager.h"

extern p_search_data sdp;

START_TEST (test_searcher_simple_sw)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_constant_scoring( 1, -1 );

        p_query query = query_read( "./tests/testdata/short_query.fas" );

        ssa_db_init_fasta( "./tests/testdata/short_db.fas" );

        init_for_sw( query, 1 );
        gapO = 1;
        gapE = 1;

        it_init( 1 );

        p_search_result res = s_search( sdp );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 1, res->seq_count );

        ck_assert_int_eq( 1, res->heap->alloc );
        ck_assert_int_eq( 1, res->heap->count );

        minheap_sort( res->heap );

        elem_t e = res->heap->array[0];
        ck_assert_int_eq( 0, e.db_id );
        ck_assert_int_eq( 0, e.query_id );

        ck_assert_int_eq( 2, e.score );

        s_free( res );
        it_free();
        mat_free();
        query_free( query );
    }END_TEST

START_TEST (test_searcher_more_sequences_sw)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_constant_scoring( 1, -1 );

        p_query query = query_read( "./tests/testdata/one_seq.fas" );

        ssa_db_init_fasta( "./tests/testdata/test.fas" );

        init_for_sw( query, 3 );
        gapO = 1;
        gapE = 1;

        it_init( 3 );
        p_search_result res = s_search( sdp );

        ck_assert_int_eq( 2, res->chunk_count );
        ck_assert_int_eq( 5, res->seq_count );

        ck_assert_int_eq( 3, res->heap->alloc );
        ck_assert_int_eq( 3, res->heap->count );

        minheap_sort( res->heap );

        elem_t e = res->heap->array[0];

        ck_assert_int_eq( 1, e.db_id );
        ck_assert_int_eq( 0, e.query_id );
        ck_assert_int_eq( 53, e.score );

        s_free( res );
        it_free();
        mat_free();
        query_free( query );
    }END_TEST

START_TEST (test_searcher_simple_nw)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_constant_scoring( 1, -1 );

        p_query query = query_read( "./tests/testdata/short_query.fas" );

        ssa_db_init_fasta( "./tests/testdata/short_db.fas" );

        init_for_nw( query, 1 );
        gapO = 1;
        gapE = 1;

        it_init( 1 );

        p_search_result res = s_search( sdp );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 1, res->seq_count );

        ck_assert_int_eq( 1, res->heap->alloc );
        ck_assert_int_eq( 1, res->heap->count );

        minheap_sort( res->heap );

        elem_t e = res->heap->array[0];
        ck_assert_int_eq( 0, e.db_id );
        ck_assert_int_eq( 0, e.query_id );

        ck_assert_int_eq( -2, e.score );

        s_free( res );
        it_free();
        mat_free();
        query_free( query );
    }END_TEST

START_TEST (test_searcher_nw_blosum62)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_buildin( BLOSUM62 );

        p_query query = query_read( "./tests/testdata/NP_009305.1.fas" );

        ssa_db_init_fasta( "./tests/testdata/AF091148.fas" );

        init_for_nw( query, 1 );
        gapO = 1;
        gapE = 1;

        it_init( 1403 );

        p_search_result res = s_search( sdp );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 1403, res->seq_count );

        ck_assert_int_eq( 1, res->heap->alloc );
        ck_assert_int_eq( 1, res->heap->count );

        minheap_sort( res->heap );

        elem_t e = res->heap->array[0];
        ck_assert_int_eq( 1245, e.db_id );
        ck_assert_int_eq( 0, e.query_id );

        ck_assert_int_eq( 219, e.score );

        s_free( res );
        it_free();
        mat_free();
        query_free( query );
    }END_TEST

START_TEST (test_searcher_more_sequences_nw)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_constant_scoring( 1, -1 );

        p_query query = query_read( "./tests/testdata/one_seq.fas" );

        ssa_db_init_fasta( "./tests/testdata/test.fas" );

        init_for_nw( query, 3 );
        gapO = 1;
        gapE = 1;

        it_init( 3 );
        p_search_result res = s_search( sdp );

        ck_assert_int_eq( 2, res->chunk_count );
        ck_assert_int_eq( 5, res->seq_count );

        ck_assert_int_eq( 3, res->heap->alloc );
        ck_assert_int_eq( 3, res->heap->count );

        minheap_sort( res->heap );

        elem_t e = res->heap->array[0];

        ck_assert_int_eq( 4, e.db_id );
        ck_assert_int_eq( 0, e.query_id );
        ck_assert_int_eq( -43, e.score );

        s_free( res );
        it_free();
        mat_free();
        query_free( query );
    }END_TEST

void addSearcherTC( Suite *s ) {
    TCase *tc_core = tcase_create( "searcher" );
    tcase_add_test( tc_core, test_searcher_simple_sw );
    tcase_add_test( tc_core, test_searcher_more_sequences_sw );
    tcase_add_test( tc_core, test_searcher_simple_nw );
    tcase_add_test( tc_core, test_searcher_nw_blosum62 );
    tcase_add_test( tc_core, test_searcher_more_sequences_nw );

    suite_add_tcase( s, tc_core );
}
