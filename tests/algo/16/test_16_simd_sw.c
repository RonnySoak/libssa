/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../../tests.h"

#include "../../../src/util/util.h"
#include "../../../src/libssa.h"
#include "../../../src/matrices.h"
#include "../../../src/db_iterator.h"
#include "../../../src/query.h"
#include "../../../src/util/minheap.h"
#include "../../../src/algo/16/search_16.h"
#include "../../../src/algo/search.h"
#include "../../../src/algo/searcher.h"

static p_search_result setup_searcher_16_test( char * query_string, char * db_file, int hit_count ) {
    mat_init_constant_scoring( 1, -1 );
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    p_query query = query_read_from_string( "short query", query_string );

    s_init( SMITH_WATERMAN, BIT_WIDTH_16, query, hit_count );

    ssa_db_init_fasta( concat( "./tests/testdata/", db_file ) );

    gapO = 1;
    gapE = 1;

    it_init( hit_count );

    p_search_result res = s_search( NULL );

    minheap_sort( res->heap );

    query_free( query );

    return res;
}

static void exit_searcher_16_test( p_search_result res ) {
    s_free( res );
    it_free();
    mat_free();
}

START_TEST (test_sw_simd_simple)
    {
        p_search_result res = setup_searcher_16_test( "AT", "short_db.fas", 1 );

        p_minheap heap = res->heap;

        ck_assert_int_eq( 2, heap->array[0].score );

        exit_searcher_16_test( res );
    }END_TEST

START_TEST (test_sw_simd_simple_2)
    {
        /*
         Q:  ATGC AAA
         DB: ATGCCCAA

         A T G C C C A A
         A1x      2
         T   x
         G     x
         C       x x x
         A x           x x
         A x           x x
         A x           x x

         Cigar: 4M - ATGC
         */
        p_search_result res = setup_searcher_16_test( "ATGCAAA", "tmp.fas", 1 );

        p_minheap heap = res->heap;

        ck_assert_int_eq( 4, heap->array[0].score );

        exit_searcher_16_test( res );
    }END_TEST

START_TEST (test_sw_simd_BLOSUM62)
    {
        ck_abort_msg( "TODO not yet implemented" );
//        p_query query = query_read_from_file( "./tests/testdata/NP_009305.1.fas" );
//
//        ssa_db_init_fasta( "./tests/testdata/short_db.fas" );
//
//        setup_searcher_16_test_init( query, 1403, NUCLEOTIDE, FORWARD_STRAND );
//        mat_init_buildin( BLOSUM62 );
//
//        gapO = 1;
//        gapE = 1;
//
//        it_init( 1403 );
//
//        p_search_result res = s_search( NULL );
//
//        minheap_sort( res->heap );
//
//        query_free( query );
//
//        p_minheap heap = res->heap;
//
//        ck_assert_int_eq( 0, heap->array[0].score ); // TODO check if correct!!
//        ck_assert_int_eq( 258, heap->array[1].score );
//        ck_assert_int_eq( 258, heap->array[2].score );
//
//        exit_searcher_16_test( res );
    }END_TEST

START_TEST (test_sw_simd_more_sequences)
    {
        p_search_result res = setup_searcher_16_test( "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA",
                "test.fas", 5 );

        ck_assert_int_eq( 4, res->heap->array[0].db_id );
        ck_assert_int_eq( 0, res->heap->array[0].query_id );
        ck_assert_int_eq( 8, res->heap->array[0].score );

        ck_assert_int_eq( 8, res->heap->array[1].score ); // TODO check if correct!!
        ck_assert_int_eq( 8, res->heap->array[2].score );
        ck_assert_int_eq( 8, res->heap->array[3].score );
        ck_assert_int_eq( 8, res->heap->array[4].score );

        exit_searcher_16_test( res );
    }END_TEST

void addSmithWaterman16TC( Suite *s ) {
    TCase *tc_core = tcase_create( "SmithWaterman16" );
    tcase_add_test( tc_core, test_sw_simd_simple );
    tcase_add_test( tc_core, test_sw_simd_simple_2 );
    tcase_add_test( tc_core, test_sw_simd_BLOSUM62 );
    tcase_add_test( tc_core, test_sw_simd_more_sequences );

    suite_add_tcase( s, tc_core );
}
