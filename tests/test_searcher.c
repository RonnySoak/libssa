/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "tests.h"

#include "../src/util.h"
#include "../src/libssa.h"
#include "../src/internal_datatypes.h"
#include "../src/util/minheap.h"

extern void mat_init_constant_scoring(const uint8_t matchscore,
        const uint8_t mismatchscore);
extern void mat_free();

extern long fullsw(int8_t * dseq,
        int8_t * dend,
        int8_t * qseq,
        int8_t * qend,
        int64_t * hearray,
        int64_t * score_matrix,
        uint8_t gapopenextend,
        uint8_t gapextend);

extern void it_init(long chunk_count);
extern void it_free();

extern void s_init(p_search_data data,
        long (* algo_p) (int8_t *, int8_t *, int8_t *, int8_t *, int64_t *, int64_t *, uint8_t, uint8_t),
        long res_count);
extern p_result s_search();
extern void s_free(p_result p);

extern p_query query_read(char* filename);
extern void query_free(p_query p);

extern p_search_data init_searchdata(p_query query);

/**
 * A small helper, to print sequences in decimal representation.
 */
void print_sequence(char* desc, int8_t* seq, long len) {
    printf("seq (%s): '", desc);
    for (int i = 0; i < len; i++) {
        printf("%d", seq[i]);
    }
    printf("'\n");
}

START_TEST (test_searcher_simple)
    {
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
        mat_init_constant_scoring(1, -1);

        p_query query = query_read("./tests/testdata/short_query.fas");

        ssa_db_init_fasta("./tests/testdata/short_db.fas");

        p_search_data sdp = init_searchdata(query);

        int res_count = 1;
        s_init(sdp, &fullsw, res_count);

        it_init(1);

        p_result res = s_search();

        ck_assert_int_eq(1, res->chunk_count);
        ck_assert_int_eq(1, res->seq_count);

        ck_assert_int_eq(1, res->heap->alloc);
        ck_assert_int_eq(1, res->heap->count);

        minheap_sort(res->heap);

        elem_t e = res->heap->array[0];
        p_sdb_sequence dseq = (p_sdb_sequence)e.db_seq;
        seq_buffer * qseq = (seq_buffer*)e.query;

        ck_assert_ptr_ne(NULL, dseq);
        ck_assert_ptr_ne(NULL, qseq);

        ck_assert_int_eq(4, (int)dseq->len);

        ck_assert_int_eq(2, e.score);

        s_free(res);
        it_free();
        mat_free();
        query_free(query);
    }END_TEST

START_TEST (test_searcher_more_sequences)
    {
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
        mat_init_constant_scoring(1, -1);

        p_query query = query_read("./tests/testdata/one_seq.fas");

        ssa_db_init_fasta("./tests/testdata/test.fas");

        p_search_data sdp = init_searchdata(query);

        int res_count = 3;
        s_init(sdp, &fullsw, res_count);

        it_init(3);
        p_result res = s_search();

        ck_assert_int_eq(2, res->chunk_count);
        ck_assert_int_eq(5, res->seq_count);

        ck_assert_int_eq(3, res->heap->alloc);
        ck_assert_int_eq(3, res->heap->count);

        minheap_sort(res->heap);

        elem_t e = res->heap->array[0];

        ck_assert_ptr_ne(NULL, e.db_seq);
        ck_assert_ptr_ne(NULL, e.query);
        ck_assert_int_eq(53, e.score);

        s_free(res);
        it_free();
        mat_free();
        query_free(query);
    }END_TEST

void addSearcherTC(Suite *s) {
    TCase *tc_core = tcase_create("searcher");
    tcase_add_test(tc_core, test_searcher_simple);
    tcase_add_test(tc_core, test_searcher_more_sequences);

    suite_add_tcase(s, tc_core);
}
