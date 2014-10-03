/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/util.h"
#include "../../src/libssa.h"

extern void mat_init_constant_scoring(const uint8_t matchscore,
        const uint8_t mismatchscore);
extern void mat_init_buildin(char* matrixname);
extern void mat_free();
extern long * score_matrix_63;

extern void it_init();
extern p_sdb_sequence it_next_sequence();
extern void it_free();

extern long fullsw(sequence * dseq, sequence * qseq,
        int64_t * hearray, int64_t * score_matrix, uint8_t gapopenextend,
        uint8_t gapextend);

extern p_query query_read(char* filename);
extern void query_free(p_query p);

START_TEST (test_search63_simple)
    {
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
        mat_init_constant_scoring(1, -1);

        p_query query = query_read("./tests/testdata/short_query.fas");

        ssa_db_init_fasta("./tests/testdata/short_db.fas");
        it_init();
        p_sdb_sequence dseq = it_next_sequence();

        long *hearray = calloc(sizeof(long), 32 * query->nt[0].len);
        uint8_t gapopenextend = 1;
        uint8_t gapextend = 1;

        int score = fullsw(&dseq->seq,
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
        it_init();
        p_sdb_sequence dseq = it_next_sequence();

        long *hearray = calloc(sizeof(long), 32 * query->nt[0].len);
        uint8_t gapopenextend = 1;
        uint8_t gapextend = 1;

        int score = fullsw(&dseq->seq,
                &query->nt[0],
                hearray,
                score_matrix_63,
                gapopenextend, gapextend);

        ck_assert_int_eq(score, 12);

        it_free();
        mat_free();
        query_free(query);
    }END_TEST

void addSearch63TC(Suite *s) {
    TCase *tc_core = tcase_create("search63");
    tcase_add_test(tc_core, test_search63_simple);
    tcase_add_test(tc_core, test_search63_simple_blosum62);

    suite_add_tcase(s, tc_core);
}
