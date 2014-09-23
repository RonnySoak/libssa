/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "tests.h"

#include "../src/util.h"
#include "../src/libsdb.h"

extern void us_init_translation(long qtableno, long dtableno);
extern char* us_remap_sequence(char* sequence, int len, const char* remap);

extern void ck_converted_prot_eq(char* ref, char* protp, int plen);

extern void it_init();
extern void it_free();
extern p_sdb_sequence it_next();

extern const char * sym_ncbi_nt16u;

/**
 * Simple reverse complement implementation for upper case nucleotide sequences.
 */
char* revcompl(char* seq, int len) {
    char* rc = (char *) xmalloc(len + 1);

    for (long i = 0; i < len; i++) {
        switch ((int) seq[len - 1 - i]) {
        case 65:
            rc[i] = 'T';
            break;
        case 67:
            rc[i] = 'G';
            break;
        case 71:
            rc[i] = 'C';
            break;
        case 84:
            case 85:
            rc[i] = 'A';
            break;
        }
    }
    rc[len] = 0;

    return rc;
}

START_TEST (test_init)
    {
        // should return doing nothing
        it_free();

        symtype = NUCLEOTIDE;
        it_init();
        it_free();

        symtype = TRANS_QUERY;
        it_init();
        it_free();

        symtype = TRANS_DB;
        it_init();
        it_free();

        symtype = TRANS_BOTH;
        it_init();
        it_free();

        symtype = AMINOACID;
        it_init();
        it_free();
    }END_TEST

START_TEST (test_next_empty)
    {
        symtype = TRANS_DB;

        it_init();

        ck_assert_ptr_eq(NULL, it_next());

        it_free();
    }END_TEST

START_TEST (test_next_one)
    {
        // test with only forward strand
        sdb_init_fasta("tests/testdata/one_seq.fas");

        symtype = NUCLEOTIDE;
        query_strands = FORWARD_STRAND;

        it_init();

        p_sdb_sequence seq = it_next();
        ck_assert_int_eq(0, seq->info->ID);
        ck_assert_int_eq(54, seq->len);
        ck_assert_str_eq(
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA",
                us_remap_sequence(seq->seq, seq->len, sym_ncbi_nt16u));
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(0, seq->frame);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next());

        it_free();
        sdb_free_db();

        // test both reverse complement strand creation
        sdb_init_fasta("tests/testdata/one_seq.fas");

        symtype = NUCLEOTIDE;
        query_strands = BOTH_STRANDS;
        it_init();

        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(54, seq->len);
        ck_assert_str_eq(
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA",
                us_remap_sequence(seq->seq, seq->len, sym_ncbi_nt16u));
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(0, seq->frame);

        // get reverse complement strand
        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(54, seq->len);
        ck_assert_str_eq(
                revcompl(
                        "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA",
                        seq->len),
                us_remap_sequence(seq->seq, seq->len, sym_ncbi_nt16u));
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(0, seq->frame);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next());

        it_free();
        sdb_free_db();

        // test with forward strand and db translation
        sdb_init_fasta("tests/testdata/one_seq.fas");

        symtype = TRANS_DB;
        query_strands = FORWARD_STRAND;

        us_init_translation(1, 3);

        it_init();

        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(18, seq->len);
        ck_converted_prot_eq("MPKTNSVEGFSSFEDDV*", seq->seq, seq->len);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(0, seq->frame);

        // 2.
        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(17, seq->len);
        ck_converted_prot_eq("CPSWMA*RGFHHLRTMY", seq->seq, seq->len);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(1, seq->frame);

        // 3.
        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(17, seq->len);
        ck_converted_prot_eq("AQAE*RRGVFIIWGRCM", seq->seq, seq->len);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(2, seq->frame);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next());

        it_free();
        sdb_free_db();

        // test with both strand and db translation
        sdb_init_fasta("tests/testdata/one_seq.fas");

        symtype = TRANS_DB;
        query_strands = BOTH_STRANDS;

        us_init_translation(1, 3);

        it_init();

        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(18, seq->len);
        ck_converted_prot_eq("MPKTNSVEGFSSFEDDV*", seq->seq, seq->len);
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(0, seq->frame);

        // 2.
        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(17, seq->len);
        ck_converted_prot_eq("CPSWMA*RGFHHLRTMY", seq->seq, seq->len);
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(1, seq->frame);

        // 3.
        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(17, seq->len);
        ck_converted_prot_eq("AQAE*RRGVFIIWGRCM", seq->seq, seq->len);
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(2, seq->frame);

        // 4.
        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(18, seq->len);
        ck_converted_prot_eq("LYIVTKWWKPTYAIQTGH", seq->seq, seq->len);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(0, seq->frame);

        // 5.
        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(17, seq->len);
        ck_converted_prot_eq("YTSSSNDENPSTTFSLG", seq->seq, seq->len);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(1, seq->frame);

        // 6.
        seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        ck_assert_int_eq(17, seq->len);
        ck_converted_prot_eq("MHRPQMMKTPTRYSAWA", seq->seq, seq->len);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(2, seq->frame);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next());

        it_free();
        sdb_free_db();
    }END_TEST

START_TEST (test_next)
    {
        sdb_init_fasta("tests/testdata/test.fas");

        // test with only forward strand
        symtype = NUCLEOTIDE;
        query_strands = FORWARD_STRAND;

        it_init();

        // 1.
        p_sdb_sequence seq = it_next();
        ck_assert_ptr_eq(0, seq->info->ID);
        // 2.
        seq = it_next();
        ck_assert_ptr_eq(1, seq->info->ID);
        // 3.
        seq = it_next();
        ck_assert_ptr_eq(2, seq->info->ID);
        // 4.
        seq = it_next();
        ck_assert_ptr_eq(3, seq->info->ID);
        // 5.
        seq = it_next();
        ck_assert_ptr_eq(4, seq->info->ID);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next());

        it_free();
        sdb_free_db();
    }END_TEST

void addDBIteratorTC(Suite *s) {
    TCase *tc_core = tcase_create("db_iterator");
    tcase_add_test(tc_core, test_init);
    tcase_add_test(tc_core, test_next_empty);
    tcase_add_test(tc_core, test_next_one);
    tcase_add_test(tc_core, test_next);

    suite_add_tcase(s, tc_core);
}

