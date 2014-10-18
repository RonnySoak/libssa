/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/util.h"
#include "../../src/libssa.h"
#include "../../src/util/minheap.h"
#include "../../src/matrices.h"
#include "../../src/db_iterator.h"
#include "../../src/query.h"
#include "../../src/algo/searcher.h"
#include "../../src/algo/search.h"

extern p_search_data init_searchdata(p_query query);

START_TEST (test_searcher_simple_sw)
    {
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
        mat_init_constant_scoring(1, -1);

        p_query query = query_read("./tests/testdata/short_query.fas");

        ssa_db_init_fasta("./tests/testdata/short_db.fas");

        p_search_data sdp = init_searchdata(query);

        int res_count = 1;
        s_init_search_algo(&full_sw);
        s_init(sdp, res_count);

        it_init(1);

        p_search_result res = s_search();

        ck_assert_int_eq(1, res->chunk_count);
        ck_assert_int_eq(1, res->seq_count);

        ck_assert_int_eq(1, res->heap->alloc);
        ck_assert_int_eq(1, res->heap->count);

        minheap_sort(res->heap);

        elem_t e = res->heap->array[0];
        ck_assert_int_eq(0, e.db_id);
        ck_assert_int_eq(0, e.query_id);

        ck_assert_int_eq(2, e.score);

        s_free(res);
        it_free();
        mat_free();
        query_free(query);
    }END_TEST

START_TEST (test_searcher_more_sequences_sw)
    {
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
        mat_init_constant_scoring(1, -1);

        p_query query = query_read("./tests/testdata/one_seq.fas");

        ssa_db_init_fasta("./tests/testdata/test.fas");

        p_search_data sdp = init_searchdata(query);

        int res_count = 3;
        s_init_search_algo(&full_sw);
        s_init(sdp, res_count);

        it_init(3);
        p_search_result res = s_search();

        ck_assert_int_eq(2, res->chunk_count);
        ck_assert_int_eq(5, res->seq_count);

        ck_assert_int_eq(3, res->heap->alloc);
        ck_assert_int_eq(3, res->heap->count);

        minheap_sort(res->heap);

        elem_t e = res->heap->array[0];

        ck_assert_int_eq(1, e.db_id);
        ck_assert_int_eq(0, e.query_id); // TODO find out
        ck_assert_int_eq(53, e.score);

        s_free(res);
        it_free();
        mat_free();
        query_free(query);
    }END_TEST

void addSearcherTC(Suite *s) {
    TCase *tc_core = tcase_create("searcher");
    tcase_add_test(tc_core, test_searcher_simple_sw);
    tcase_add_test(tc_core, test_searcher_more_sequences_sw);

    suite_add_tcase(s, tc_core);
}
