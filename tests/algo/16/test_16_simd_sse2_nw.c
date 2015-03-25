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
#include "../../../src/matrices.h"
#include "../../../src/cpu_config.h"
#include "../../../src/query.h"
#include "../../../src/util/minheap.h"
#include "../../../src/algo/16/search_16.h"
#include "../../../src/algo/gap_costs.h"
#include "../../../src/algo/searcher.h"
#include "../../../src/db_adapter.h"

static p_search_result setup_searcher_16_test( char * query_string, char * db_file, size_t hit_count ) {
    set_max_compute_capability( COMPUTE_ON_SSE2 );

    mat_init_constant_scoring( 1, -1 );
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    p_query query = query_read_from_string( query_string );

    s_init( NEEDLEMAN_WUNSCH, BIT_WIDTH_16, query );

    ssa_db_init( concat( "./tests/testdata/", db_file ) );

    gapO = 1;
    gapE = 1;

    adp_init( hit_count );

    p_search_result res = s_search( &hit_count );

    minheap_sort( res->heap );

    query_free( query );

    return res;
}

static void exit_searcher_16_test( p_search_result res ) {
    s_free( res );
    adp_exit();
    mat_free();

    reset_compute_capability();
}

START_TEST (test_nw_simd_simple)
    {
        p_search_result res = setup_searcher_16_test( "AT", "short_db.fas", 1 );

        p_minheap heap = res->heap;

        ck_assert_int_eq( -2, heap->array[0].score );

        exit_searcher_16_test( res );
    }END_TEST

START_TEST (test_nw_simd_more_sequences)
    {
        p_search_result res = setup_searcher_16_test( "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA",
                "test.fas", 5 );

        p_minheap heap = res->heap;

        ck_assert_int_eq( -43, heap->array[0].score );
        ck_assert_int_eq( -50, heap->array[1].score );
        ck_assert_int_eq( -52, heap->array[2].score );
        ck_assert_int_eq( -52, heap->array[3].score );
        ck_assert_int_eq( -147, heap->array[4].score );

        exit_searcher_16_test( res );
    }END_TEST

void add_nw_16_SSE2_TC( Suite *s ) {
    TCase *tc_core = tcase_create( "NeedlemanWunsch_16_SSE2" );
    tcase_add_test( tc_core, test_nw_simd_simple );
    tcase_add_test( tc_core, test_nw_simd_more_sequences );

    suite_add_tcase( s, tc_core );
}
