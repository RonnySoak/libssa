/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "tests.h"

#include "../src/util.h"
#include "../src/libssa.h"

extern void mat_init_buildin(char* matrixname);
extern void mat_free();
extern long * score_matrix_63;

extern long fullsw(char * dseq, char * dend, char * qseq, char * qend,
        long * hearray, long * score_matrix, uint8_t gapopenextend,
        uint8_t gapextend);

START_TEST (test_search63_same_seqs)
    {
        // TODO move to init function
        mat_init_buildin(BLOSUM62);

        char* dseq = "ATAT";
        char* dend = &dseq[1];
        char* qseq = "AT";
        char* qend = &qseq[1];
        long *hearray = calloc(sizeof(long), 2 * (qend - qseq));
        uint8_t gapopenextend = 4;
        uint8_t gapextend = 2;

        printf("value 1: %d\n", (int) score_matrix_63[1]);

        int score = fullsw(dseq, dend, qseq, qend, hearray, score_matrix_63,
                gapopenextend, gapextend);
        printf("score: %d\n", score);

        ck_assert_int_eq(score, 0);

        mat_free();
    }END_TEST

void addSearch63TC(Suite *s) {
    TCase *tc_core = tcase_create("search63");
    tcase_add_test(tc_core, test_search63_same_seqs);

    suite_add_tcase(s, tc_core);
}
