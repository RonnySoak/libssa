/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/util/util.h"
#include "../../src/libssa.h"
#include "../../src/matrices.h"
#include "../../src/db_iterator.h"
#include "../../src/query.h"

extern long full_sw(sequence * dseq, sequence * qseq,
        int64_t * hearray, int64_t * score_matrix, uint8_t gapopenextend,
        uint8_t gapextend);

START_TEST (test_search63_simple)
    {
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
        mat_init_constant_scoring(1, -1);

        p_query query = query_read("./tests/testdata/short_query.fas");

        ssa_db_init_fasta("./tests/testdata/short_db.fas");
        it_init(1);
        sequence dseq = it_translate_sequence(ssa_db_get_sequence(0), 0, 0);

        long *hearray = calloc(sizeof(long), 32 * query->nt[0].len);
        uint8_t gapopenextend = 1;
        uint8_t gapextend = 1;

        int score = full_sw(&dseq,
                &query->nt[0],
                hearray,
                score_matrix_63,
                gapopenextend, gapextend);

        ck_assert_int_eq(score, 2);

        it_free();
        mat_free();
        query_free(query);
    }END_TEST

START_TEST (test_search63_simple_blosum62)
    {
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
        mat_init_buildin(BLOSUM62);

        p_query query = query_read("./tests/testdata/short_query.fas");

        ssa_db_init_fasta("./tests/testdata/short_db.fas");
        it_init(1);
        sequence dseq = it_translate_sequence(ssa_db_get_sequence(0), 0, 0);

        long *hearray = calloc(sizeof(long), 32 * query->nt[0].len);
        uint8_t gapopenextend = 1;
        uint8_t gapextend = 1;

        int score = full_sw(&dseq,
                &query->nt[0],
                hearray,
                score_matrix_63,
                gapopenextend, gapextend);

        ck_assert_int_eq(score, 12);

        it_free();
        mat_free();
        query_free(query);
    }END_TEST

void addSmithWaterman63TC(Suite *s) {
    TCase *tc_core = tcase_create("SmithWaterman63");
    tcase_add_test(tc_core, test_search63_simple);
    tcase_add_test(tc_core, test_search63_simple_blosum62);

    suite_add_tcase(s, tc_core);
}
