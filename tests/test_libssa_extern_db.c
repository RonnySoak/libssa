/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "tests.h"

#include "../src/libssa_extern_db.h"

START_TEST (test_one_sequence)
	{
		ssa_db_init("tests/testdata/one_seq.fas");

		p_seqinfo seq = ssa_db_get_sequence( 0 );
		ck_assert_int_eq(54, seq->seqlen);

		ck_assert_ptr_eq(NULL, ssa_db_get_sequence( 1 ) );

		ssa_db_close();
	}END_TEST

START_TEST (test_multiple_sequences)
	{
		ssa_db_init("tests/testdata/AF091148.fas");

		ck_assert_int_eq(1403, ssa_db_get_sequence_count());

		// checks if it can read all sequences
		for (int i = 0; i < 1403; i++) {
			p_seqinfo ref_seq = ssa_db_get_sequence( i );
			ck_assert_ptr_ne(NULL, ref_seq);

			ck_assert_int_eq(i, (int )ref_seq->ID);
		}

		ck_assert_ptr_eq(NULL, ssa_db_get_sequence( 1403 ) );

		// end reached?
        ck_assert_ptr_eq(NULL, ssa_db_get_sequence( 1404 ) );

		ssa_db_close();
	}END_TEST

START_TEST (test_get_sequence)
	{
		ssa_db_init("tests/testdata/test.fas");

		ck_assert_int_eq(5, ssa_db_get_sequence_count());

		ck_assert_int_eq(0, ssa_db_get_sequence(0)->ID);
		ck_assert_int_eq(1, ssa_db_get_sequence(1)->ID);
		ck_assert_int_eq(4, ssa_db_get_sequence(4)->ID);
		ck_assert_ptr_eq(NULL, ssa_db_get_sequence(5));

		ssa_db_close();
    }END_TEST

void addLibSSAExternDBTC(Suite *s) {
	TCase *tc_core = tcase_create("libssa_extern_db");
	tcase_add_test(tc_core, test_one_sequence);
	tcase_add_test(tc_core, test_multiple_sequences);
	tcase_add_test(tc_core, test_get_sequence);

	suite_add_tcase(s, tc_core);
}

