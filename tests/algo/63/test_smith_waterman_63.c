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
#include "../../../src/algo/search.h"
#include "../../../src/algo/searcher.h"

#include "../../../src/algo/63/search_63.h"

START_TEST (test_search63_simple)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_constant_scoring( 1, -1 );

        p_query query = query_read_from_string( "short query", "AT" );

        ssa_db_init_fasta( "./tests/testdata/short_db.fas" );
        it_init( 1 );
        sequence dseq = it_translate_sequence( ssa_db_get_sequence( 0 ), 0, 0 );

        long *hearray = calloc( sizeof(long), 32 * query->nt[0].len );
        gapO = 1;
        gapE = 1;

        int score = full_sw( &dseq, &query->nt[0], hearray );

        ck_assert_int_eq( score, 2 );

        it_free();
        mat_free();
        query_free( query );
    }END_TEST

START_TEST (test_search63_simple_2)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_constant_scoring( 1, -1 );

        p_query query = query_read_from_string( "query", "ATGCAAA" );

        ssa_db_init_fasta( "./tests/testdata/tmp.fas" );
        it_init( 1 );
        sequence dseq = it_translate_sequence( ssa_db_get_sequence( 0 ), 0, 0 );

        long *hearray = calloc( sizeof(long), 32 * query->nt[0].len );
        gapO = 1;
        gapE = 1;

        int score = full_sw( &dseq, &query->nt[0], hearray );

        ck_assert_int_eq( score, 4 );

        it_free();
        mat_free();
        query_free( query );
    }END_TEST

START_TEST (test_search63_simple_blosum62)
    {
        init_symbol_translation( TRANS_DB, FORWARD_STRAND, 3, 3 );
        mat_init_buildin( BLOSUM62 );

        p_query query = query_read_from_file( "./tests/testdata/NP_009305.1.fas" );

        ssa_db_init_fasta( "./tests/testdata/AF091148.fas" );

        s_init( SMITH_WATERMAN, BIT_WIDTH_64, query, 3 );

        gapO = 1;
        gapE = 1;

        it_init( 1403 );

        p_search_result res = s_search( NULL );

        minheap_sort( res->heap );

        query_free( query );

        p_minheap heap = res->heap;

        ck_assert_int_eq( 69, heap->array[0].score );
        ck_assert_int_eq( 271, heap->array[0].db_id );
        ck_assert_int_eq( 68, heap->array[1].score );
        ck_assert_int_eq( 1074, heap->array[1].db_id);
        ck_assert_int_eq( 68, heap->array[2].score );
        ck_assert_int_eq( 921, heap->array[2].db_id );

        s_free( res );
        it_free();
        mat_free();
    }END_TEST

void addSmithWaterman63TC( Suite *s ) {
    TCase *tc_core = tcase_create( "SmithWaterman63" );
    tcase_add_test( tc_core, test_search63_simple );
    tcase_add_test( tc_core, test_search63_simple_2 );
    tcase_add_test( tc_core, test_search63_simple_blosum62 );

    suite_add_tcase( s, tc_core );
}
