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

#include "../../../src/algo/64/search_64.h"
#include "../../tests.h"

#include "../../../src/util/util.h"
#include "../../../src/util/util_sequence.h"
#include "../../../src/libssa.h"
#include "../../../src/matrices.h"
#include "../../../src/query.h"
#include "../../../src/algo/searcher.h"

#include "../../../src/algo/gap_costs.h"
#include "../../../src/db_adapter.h"

START_TEST (test_sw_64_simple)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_constant_scoring( 1, -1 );

        p_query query = query_read_from_string( "short query", "AT" );

        sequence_t dseq = us_prepare_sequence( "AATG", 4, 0, 0 );

        long *hearray = calloc( sizeof(long), 32 * query->nt[0].len );
        gapO = 1;
        gapE = 1;

        int score = full_sw( &dseq, &query->nt[0], hearray );

        ck_assert_int_eq( score, 2 );

        mat_free();
        query_free( query );
    }END_TEST

START_TEST (test_sw_64_simple_2)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        mat_init_constant_scoring( 1, -1 );

        p_query query = query_read_from_string( "query", "ATGCAAA" );

        sequence_t dseq = us_prepare_sequence( "ATGCCCAA", 4, 0, 0 );

        long *hearray = calloc( sizeof(long), 32 * query->nt[0].len );
        gapO = 1;
        gapE = 1;

        int score = full_sw( &dseq, &query->nt[0], hearray );

        ck_assert_int_eq( score, 4 );

        mat_free();
        query_free( query );
    }END_TEST

void add_sw_64_TC( Suite *s ) {
    TCase *tc_core = tcase_create( "SmithWaterman_64" );
    tcase_add_test( tc_core, test_sw_64_simple );
    tcase_add_test( tc_core, test_sw_64_simple_2 );

    suite_add_tcase( s, tc_core );
}
