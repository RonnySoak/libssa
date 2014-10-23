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
#include "../../src/util_sequence.h"

sequence a_seq;
sequence b_seq;

void fill_translated_sequence(sequence* s, char * seq, long len) {
    sequence new_seq = { seq, len };
    *s = us_map_sequence(new_seq, map_ncbi_nt16);

    // TODO move to test util file
}

void setup_align(char * seq_a, char * seq_b, long len_a, long len_b) {
    gapO = 1;
    gapE = 1;
    mat_init_constant_scoring(1, -1);

    us_init_translation(3, 3);

    fill_translated_sequence(&a_seq, seq_a, len_a);
    fill_translated_sequence(&b_seq, seq_b, len_b);
}

void teardown_align() {
    free(a_seq.seq);
    free(b_seq.seq);

    mat_free();
}

START_TEST (test_find_region_for_local)
    {
        setup_align("AATC", "AT", 4, 2);

        region_t region = find_region_and_score_for_local(a_seq, b_seq);

        ck_assert_int_eq(1, region.a_begin);
        ck_assert_int_eq(2, region.a_end);
        ck_assert_int_eq(0, region.b_begin);
        ck_assert_int_eq(1, region.b_end);

        teardown_align();
    }END_TEST

START_TEST (test_find_region_for_global)
    {
        setup_align("AATC", "AT", 4, 2);

        region_t region = init_region_for_global(a_seq, b_seq);

        ck_assert_int_eq(0, region.a_begin);
        ck_assert_int_eq(3, region.a_end);
        ck_assert_int_eq(0, region.b_begin);
        ck_assert_int_eq(1, region.b_end);

        teardown_align();
    }END_TEST

void addAlignTC(Suite *s) {
    TCase *tc_core = tcase_create("align");
    tcase_add_test(tc_core, test_find_region_for_local);
    tcase_add_test(tc_core, test_find_region_for_global);

    suite_add_tcase(s, tc_core);
}