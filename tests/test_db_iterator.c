/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "tests.h"

#include "../src/util.h"
#include "../src/libssa_extern_db.h"

extern void us_init_translation(int qtableno, int dtableno);
extern sequence us_map_sequence(sequence seq, const char* remap);

extern void ck_converted_prot_eq(char* ref, sequence seq);

extern void it_init(unsigned long size);
extern void it_free();
extern void it_free_chunk(p_db_chunk chunk);
extern p_sdb_sequence it_next_sequence();
extern p_db_chunk it_next_chunk();
extern void it_free_sequence(p_sdb_sequence seq);

extern const char * sym_ncbi_nt16u;

/**
 * Simple reverse complement implementation for upper case nucleotide sequences.
 */
char* revcompl(char* seq, unsigned long len) {
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
        it_init(1);
        it_free();

        symtype = TRANS_QUERY;
        it_init(1);
        it_free();

        symtype = TRANS_DB;
        it_init(1);
        it_free();

        symtype = TRANS_BOTH;
        it_init(1);
        it_free();

        symtype = AMINOACID;
        it_init(1);
        it_free();
    }END_TEST

START_TEST (test_next_empty)
    {
        symtype = TRANS_DB;

        it_init(1);

        ck_assert_ptr_eq(NULL, it_next_sequence());

        it_free();
    }END_TEST

START_TEST (test_next_one)
    {
        // test with only forward strand
        ssa_db_init_fasta("tests/testdata/one_seq.fas");

        symtype = NUCLEOTIDE;
        query_strands = FORWARD_STRAND;

        it_init(1);

        p_sdb_sequence seq = it_next_sequence();
        ck_assert_int_eq(0, seq->ID);
        ck_assert_int_eq(54, seq->seq.len);
        ck_assert_str_eq(
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA",
                us_map_sequence(seq->seq, sym_ncbi_nt16u).seq);
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(0, seq->frame);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next_sequence());

        it_free();
        ssa_db_free();

        // test both reverse complement strand creation
        ssa_db_init_fasta("tests/testdata/one_seq.fas");

        symtype = NUCLEOTIDE;
        query_strands = BOTH_STRANDS;
        it_init(1);

        seq = it_next_sequence();
        ck_assert_int_eq(0, seq->ID);
        ck_assert_int_eq(54, seq->seq.len);
        ck_assert_str_eq(
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA",
                us_map_sequence(seq->seq, sym_ncbi_nt16u).seq);
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(0, seq->frame);
        it_free_sequence(seq);

        // get reverse complement strand
        seq = it_next_sequence();
        ck_assert_int_eq(0, seq->ID);
        ck_assert_int_eq(54, seq->seq.len);
        ck_assert_str_eq(
                revcompl(
                        "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA",
                        seq->seq.len),
                us_map_sequence(seq->seq, sym_ncbi_nt16u).seq);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(0, seq->frame);
        it_free_sequence(seq);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next_sequence());

        it_free();
        ssa_db_free();

        // test with forward strand and db translation
        ssa_db_init_fasta("tests/testdata/one_seq.fas");

        symtype = TRANS_DB;
        query_strands = FORWARD_STRAND;

        us_init_translation(1, 3);

        it_init(1);

        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(18, seq->seq.len);
        ck_converted_prot_eq("MPKTNSVEGFSSFEDDV*", seq->seq);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(0, seq->frame);
        it_free_sequence(seq);

        // 2.
        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(17, seq->seq.len);
        ck_converted_prot_eq("CPSWMA*RGFHHLRTMY", seq->seq);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(1, seq->frame);
        it_free_sequence(seq);

        // 3.
        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(17, seq->seq.len);
        ck_converted_prot_eq("AQAE*RRGVFIIWGRCM", seq->seq);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(2, seq->frame);
        it_free_sequence(seq);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next_sequence());

        it_free();
        ssa_db_free();

        // test with both strand and db translation
        ssa_db_init_fasta("tests/testdata/one_seq.fas");

        symtype = TRANS_DB;
        query_strands = BOTH_STRANDS;

        us_init_translation(1, 3);

        it_init(1);

        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(18, seq->seq.len);
        ck_converted_prot_eq("MPKTNSVEGFSSFEDDV*", seq->seq);
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(0, seq->frame);
        it_free_sequence(seq);

        // 2.
        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(17, seq->seq.len);
        ck_converted_prot_eq("CPSWMA*RGFHHLRTMY", seq->seq);
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(1, seq->frame);
        it_free_sequence(seq);

        // 3.
        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(17, seq->seq.len);
        ck_converted_prot_eq("AQAE*RRGVFIIWGRCM", seq->seq);
        ck_assert_int_eq(0, seq->strand);
        ck_assert_int_eq(2, seq->frame);
        it_free_sequence(seq);

        // 4.
        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(18, seq->seq.len);
        ck_converted_prot_eq("LYIVTKWWKPTYAIQTGH", seq->seq);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(0, seq->frame);
        it_free_sequence(seq);

        // 5.
        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(17, seq->seq.len);
        ck_converted_prot_eq("YTSSSNDENPSTTFSLG", seq->seq);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(1, seq->frame);

        // 6.
        seq = it_next_sequence();
        ck_assert_ptr_eq(0, seq->ID);
        ck_assert_int_eq(17, seq->seq.len);
        ck_converted_prot_eq("MHRPQMMKTPTRYSAWA", seq->seq);
        ck_assert_int_eq(1, seq->strand);
        ck_assert_int_eq(2, seq->frame);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next_sequence());

        it_free();
        ssa_db_free();
    }END_TEST

START_TEST (test_next)
    {
        ssa_db_init_fasta("tests/testdata/test.fas");

        // test with only forward strand
        symtype = NUCLEOTIDE;
        query_strands = FORWARD_STRAND;

        it_init(1);

        // 1.
        p_sdb_sequence seq = it_next_sequence();
        ck_assert_int_eq(0, seq->ID);
        it_free_sequence(seq);
        // 2.
        seq = it_next_sequence();
        ck_assert_int_eq(1, seq->ID);
        it_free_sequence(seq);
        // 3.
        seq = it_next_sequence();
        ck_assert_int_eq(2, seq->ID);
        it_free_sequence(seq);
        // 4.
        seq = it_next_sequence();
        ck_assert_int_eq(3, seq->ID);
        it_free_sequence(seq);
        // 5.
        seq = it_next_sequence();
        ck_assert_int_eq(4, seq->ID);
        it_free_sequence(seq);

        // check for the end of sequences
        ck_assert_ptr_eq(NULL, it_next_sequence());

        it_free();
        ssa_db_free();
    }END_TEST

START_TEST (test_next_chunk)
    {
        ssa_db_init_fasta("tests/testdata/test.fas");

        // test with only forward strand
        symtype = NUCLEOTIDE;
        query_strands = FORWARD_STRAND;

        int chunk_size = 3;

        it_init(chunk_size);

        p_db_chunk chunk = it_next_chunk();

        // 1.
        ck_assert_int_eq(chunk_size, (int)chunk->size);
        ck_assert_int_eq(0, chunk->seq[0]->ID);
        ck_assert_int_eq(1, chunk->seq[1]->ID);
        ck_assert_int_eq(2, chunk->seq[2]->ID);
        it_free_chunk(chunk);

        // 2.
        chunk = it_next_chunk();

        ck_assert_int_eq(2, (int)chunk->size);
        ck_assert_int_eq(3, chunk->seq[0]->ID);
        ck_assert_int_eq(4, chunk->seq[1]->ID);

        it_free_chunk(chunk);

        // check for the end of sequences
        chunk = it_next_chunk();

        ck_assert_ptr_eq(NULL, chunk);

        it_free_chunk(chunk);

        it_free();
        ssa_db_free();
    }END_TEST



void addDBIteratorTC(Suite *s) {
    TCase *tc_core = tcase_create("db_iterator");
    tcase_add_test(tc_core, test_init);
    tcase_add_test(tc_core, test_next_empty);
    tcase_add_test(tc_core, test_next_one);
    tcase_add_test(tc_core, test_next);
    tcase_add_test(tc_core, test_next_chunk);

    suite_add_tcase(s, tc_core);
}

