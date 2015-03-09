/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include <string.h>

#include "../../src/algo/align.h"
#include "../../src/algo/searcher.h"
#include "../../src/matrices.h"
#include "../../src/util/util_sequence.h"
#include "../../src/algo/gap_costs.h"

extern void fill_translated_sequence(sequence_t* s, char * seq, long len);

sequence_t a_seq;
sequence_t b_seq;

void setup_cigar(char * seq_a, char * seq_b, long len_a, long len_b) {
    gapO = 1;
    gapE = 1;
    mat_init_constant_scoring(1, -1);

    us_init_translation(3, 3);

    fill_translated_sequence(&a_seq, seq_a, len_a);
    fill_translated_sequence(&b_seq, seq_b, len_b);
}

void teardown_cigar(cigar_p cigar) {
    free(a_seq.seq);
    free(b_seq.seq);

    mat_free();

    free(cigar->cigar);
    free(cigar);
}

START_TEST (test_generate_cigar_simple_nw)
    {
        setup_cigar("AATG", "AT", 4, 2);

        cigar_p cigar = compute_cigar_for_nw(a_seq, b_seq);

        ck_assert_int_eq(4, cigar->len);
        ck_assert_int_eq(5, cigar->allocated_size);
        ck_assert_str_eq("MDMD", cigar->cigar);

        teardown_cigar(cigar);

        // test size of sequences switched
        setup_cigar("AT", "AATG", 2, 4);

        cigar = compute_cigar_for_nw(a_seq, b_seq);

        ck_assert_int_eq(4, cigar->len);
        ck_assert_int_eq(5, cigar->allocated_size);
        ck_assert_str_eq("MIMI", cigar->cigar);

        teardown_cigar(cigar);

        // test longer sequences
        setup_cigar("TACGGGTAT", "GGACGTACG", 9, 9);

        cigar = compute_cigar_for_nw(a_seq, b_seq);

        ck_assert_int_eq(9, cigar->len);
        ck_assert_int_eq(10, cigar->allocated_size);
        ck_assert_str_eq("MI3M2D3MI", cigar->cigar);

        teardown_cigar(cigar);
    }END_TEST

START_TEST (test_generate_cigar_simple_sw)
    {
        setup_cigar("AATG", "AT", 4, 2);

        region_t region = find_region_and_score_for_local(a_seq, b_seq);

        cigar_p cigar = compute_cigar_for_sw(a_seq, b_seq, region);

        ck_assert_int_eq(2, cigar->len);
        ck_assert_int_eq(3, cigar->allocated_size);
        ck_assert_str_eq("2M", cigar->cigar);

        teardown_cigar(cigar);

        // test size of sequences switched
        setup_cigar("AT", "AATG", 2, 4);

        region = find_region_and_score_for_local(a_seq, b_seq);

        cigar = compute_cigar_for_sw(a_seq, b_seq, region);

        ck_assert_int_eq(2, cigar->len);
        ck_assert_int_eq(3, cigar->allocated_size);
        ck_assert_str_eq("2M", cigar->cigar);

        teardown_cigar(cigar);
        // ---
        setup_cigar("TACGGGTAT", "GGACGTACG", 9, 9);

        region = find_region_and_score_for_local(a_seq, b_seq);

        cigar = compute_cigar_for_sw(a_seq, b_seq, region);

        ck_assert_int_eq(2, cigar->len);
        ck_assert_int_eq(3, cigar->allocated_size);
        ck_assert_str_eq("4M", cigar->cigar);

        teardown_cigar(cigar);
    }END_TEST

void addCigarTC(Suite *s) {
    TCase *tc_core = tcase_create("cigar");
    tcase_add_test(tc_core, test_generate_cigar_simple_nw);
    tcase_add_test(tc_core, test_generate_cigar_simple_sw);

    suite_add_tcase(s, tc_core);
}
