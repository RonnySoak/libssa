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

#include "../../tests.h"

#include "../../../src/util/util.h"
#include "../../../src/libssa.h"
#include "../../../src/util/minheap.h"
#include "../../../src/matrices.h"
#include "../../../src/query.h"
#include "../../../src/algo/searcher.h"
#include "../../../src/algo/gap_costs.h"
#include "../../../src/db_adapter.h"

static p_query setup_searcher_test_init( int bit_width, int search_type, char * query_string, int symtype, int strands ) {
    init_symbol_translation( symtype, strands, 3, 3 );
    mat_init_constant_scoring( 1, -1 );

    p_query query = query_read_from_string( "short query", query_string );

    s_init( search_type, bit_width, query );

    return query;
}

static p_search_result setup_searcher_test( int bit_width, int search_type, char * query_string, char * db_file,
        size_t hit_count ) {
    p_query query = setup_searcher_test_init( bit_width, search_type, query_string, NUCLEOTIDE, FORWARD_STRAND );

    ssa_db_init( concat( "./tests/testdata/", db_file ) );

    gapO = 1;
    gapE = 1;

    adp_init( hit_count );

    p_search_result res = s_search( &hit_count );

    minheap_sort( res->heap );

    query_free( query );

    return res;
}

static void exit_searcher_test( p_search_result res ) {
    s_free( res );
    adp_exit();
    mat_free();
}

START_TEST (test_search_64_more_sequences_sw)
    {
        p_search_result res = setup_searcher_test( BIT_WIDTH_64, SMITH_WATERMAN,
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA", "test.fas", 5 );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 5, res->seq_count );

        ck_assert_int_eq( 5, res->heap->alloc );
        ck_assert_int_eq( 5, res->heap->count );

        ck_assert_int_eq( 4, res->heap->array[0].db_id );
        ck_assert_int_eq( 0, res->heap->array[0].query_id );
        ck_assert_int_eq( 8, res->heap->array[0].score );

        ck_assert_int_eq( 8, res->heap->array[1].score );
        ck_assert_int_eq( 8, res->heap->array[2].score );
        ck_assert_int_eq( 8, res->heap->array[3].score );
        ck_assert_int_eq( 8, res->heap->array[4].score );

        exit_searcher_test( res );
    }END_TEST

START_TEST (test_search_64_more_sequences_nw)
    {
        p_search_result res = setup_searcher_test( BIT_WIDTH_64, NEEDLEMAN_WUNSCH,
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA", "test.fas", 5 );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 5, res->seq_count );

        ck_assert_int_eq( 5, res->heap->alloc );
        ck_assert_int_eq( 5, res->heap->count );

        elem_t e = res->heap->array[0];

        ck_assert_int_eq( 4, e.db_id );
        ck_assert_int_eq( 0, e.query_id );
        ck_assert_int_eq( -43, e.score );

        ck_assert_int_eq( -50, res->heap->array[1].score );
        ck_assert_int_eq( -52, res->heap->array[2].score );
        ck_assert_int_eq( -52, res->heap->array[3].score );
        ck_assert_int_eq( -147, res->heap->array[4].score );

        exit_searcher_test( res );
    }END_TEST

void addSearcher64TC( Suite *s ) {
    TCase *tc_core = tcase_create( "searcher 64" );
    tcase_add_test( tc_core, test_search_64_more_sequences_sw );
    tcase_add_test( tc_core, test_search_64_more_sequences_nw );

    suite_add_tcase( s, tc_core );
}
