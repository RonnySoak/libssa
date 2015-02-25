/*
 * test_query.c
 *
 *  Created on: 03 Sep 2014
 *      Author: Jakob Frielingsdorf
 */

#include "tests.h"

#include "../src/libssa.h"
#include "../src/libssa_datatypes.h"
#include "../src/util/util.h"
#include "../src/matrices.h"

START_TEST (test_free)
    {
        ck_assert_ptr_eq(NULL, score_matrix_8);
        ck_assert_ptr_eq(NULL, score_matrix_16);
        ck_assert_ptr_eq(NULL, score_matrix_64);

        mat_init_buildin(BLOSUM62);

        ck_assert_ptr_ne(NULL, score_matrix_8);
        ck_assert_ptr_ne(NULL, score_matrix_16);
        ck_assert_ptr_ne(NULL, score_matrix_64);

        mat_free();

        ck_assert_ptr_eq(NULL, score_matrix_8);
        ck_assert_ptr_eq(NULL, score_matrix_16);
        ck_assert_ptr_eq(NULL, score_matrix_64);
    }END_TEST

START_TEST (test_buildin)
    {
        // Checks the first two values for the first line of each matrix

        mat_init_buildin(BLOSUM45);
        ck_assert_int_eq(5, (int)score_matrix_64[33]);
        ck_assert_int_eq(-2, (int)score_matrix_64[48]);
        ck_assert_int_eq(5, (int)score_matrix_8[33]);
        ck_assert_int_eq(-2, (int)score_matrix_8[48]);
        ck_assert_int_eq(5, (int)score_matrix_16[33]);
        ck_assert_int_eq(-2, (int)score_matrix_16[48]);
        mat_free();

        mat_init_buildin(BLOSUM50);
        ck_assert_int_eq(5, (int)score_matrix_64[33]);
        ck_assert_int_eq(-2, (int)score_matrix_64[48]);
        mat_free();

        mat_init_buildin(BLOSUM62);
        ck_assert_int_eq(4, (int)score_matrix_64[33]);
        ck_assert_int_eq(-1, (int)score_matrix_64[48]);
        mat_free();

        mat_init_buildin(BLOSUM80);
        ck_assert_int_eq(5, (int)score_matrix_64[33]);
        ck_assert_int_eq(-2, (int)score_matrix_64[48]);
        mat_free();

        mat_init_buildin(BLOSUM90);
        ck_assert_int_eq(5, (int)score_matrix_64[33]);
        ck_assert_int_eq(-2, (int)score_matrix_64[48]);
        mat_free();

        mat_init_buildin(PAM30);
        ck_assert_int_eq(6, (int)score_matrix_64[33]);
        ck_assert_int_eq(-7, (int)score_matrix_64[48]);
        mat_free();

        mat_init_buildin(PAM70);
        ck_assert_int_eq(5, (int)score_matrix_64[33]);
        ck_assert_int_eq(-4, (int)score_matrix_64[48]);
        mat_free();

        mat_init_buildin(PAM250);
        ck_assert_int_eq(2, (int)score_matrix_64[33]);
        ck_assert_int_eq(-2, (int)score_matrix_64[48]);
        mat_free();
    }END_TEST

START_TEST (test_init_from_file)
    {
        mat_init_from_file("tests/testdata/blosum90.txt");
        ck_assert_int_eq(5, (int)score_matrix_64[33]);
        ck_assert_int_eq(-2, (int)score_matrix_64[48]);
        mat_free();
    }END_TEST

START_TEST (test_init_from_string)
    {
        mat_init_from_string(mat_blosum80);
        ck_assert_int_eq(5, (int)score_matrix_64[33]);
        ck_assert_int_eq(-2, (int)score_matrix_64[48]);
        mat_free();
    }END_TEST

START_TEST (test_init_constant_scoring)
    {
        mat_init_constant_scoring(4, -2);

        ck_assert_int_eq(-1, (int)score_matrix_64[32]);
        ck_assert_int_eq(4, (int)score_matrix_64[33]);
        ck_assert_int_eq(-2, (int)score_matrix_64[34]);
        ck_assert_int_eq(-1, (int)score_matrix_64[64]);
        ck_assert_int_eq(4, (int)score_matrix_64[66]);

        mat_free();
    }END_TEST

void addMatricesTC(Suite *s) {
    TCase *tc_core = tcase_create("matrices");
    tcase_add_test(tc_core, test_free);
    tcase_add_test(tc_core, test_buildin);
    tcase_add_test(tc_core, test_init_from_file);
    tcase_add_test(tc_core, test_init_from_string);
    tcase_add_test(tc_core, test_init_constant_scoring);

    suite_add_tcase(s, tc_core);
}
