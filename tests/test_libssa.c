/*
 * test_query->c
 *
 *  Created on: 03 Sep 2014
 *      Author: Jakob Frielingsdorf
 */

#include "tests.h"

#include "../src/libssa.h"
#include "../src/query.h"
#include "../src/matrices.h"
#include "../src/algo/searcher.h"

START_TEST (test_simple_one_thread)
    {
        init_score_matrix(BLOSUM62);
        init_gap_penalties(4, 2);
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);

        set_threads(1);

        init_db_fasta("tests/testdata/AF091148.fas");

        p_query query = init_sequence_fasta("tests/testdata/one_seq.fas");

        p_alignment_list alist = sw_align(query, 5);
        ck_assert_int_eq(5, alist->len);

        ck_assert_int_eq(1290, alist->alignments[0]->db_seq.ID);
        ck_assert_int_eq(212, alist->alignments[0]->score);
        ck_assert_str_eq("7M2I3MD3MD3M2D9M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[0]->alignment);
        ck_assert_int_eq(1075, alist->alignments[1]->db_seq.ID);
        ck_assert_int_eq(212, alist->alignments[1]->score);
        ck_assert_str_eq("7M2I3MD3MD3M2D9M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[1]->alignment);
        ck_assert_int_eq(1337, alist->alignments[2]->db_seq.ID);
        ck_assert_int_eq(210, alist->alignments[2]->score);
        ck_assert_str_eq("7M2I3MD3MD4M2D8M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[2]->alignment);
        ck_assert_int_eq(1164, alist->alignments[3]->db_seq.ID);
        ck_assert_int_eq(210, alist->alignments[3]->score);
        ck_assert_str_eq("7M2I3MD3MD4M2D8M5I4M8I7MI5MI3M4I2MI2MI2M", alist->alignments[3]->alignment);
        ck_assert_int_eq(1146, alist->alignments[4]->db_seq.ID);
        ck_assert_int_eq(210, alist->alignments[4]->score);
        ck_assert_str_eq("7M2I3MD3MD4M2D8M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[4]->alignment);

        free_alignment(alist);

        alist = nw_align(query, 5);
        ck_assert_int_eq(5, alist->len);

        ck_assert_int_eq(1050, alist->alignments[0]->db_seq.ID);
        ck_assert_int_eq(112, alist->alignments[0]->score);
        ck_assert_str_eq("5M2I5M3I8M3I2M15I4M3IMI3M5I4M8I7MI5MI3M4I2MI3MI2M7I", alist->alignments[0]->alignment);
        ck_assert_int_eq(378, alist->alignments[1]->db_seq.ID);
        ck_assert_int_eq(100, alist->alignments[1]->score);
        ck_assert_str_eq("5M2I5M3I8M7IM8IM8I2M3I6M5I4M8I7MI5MI3M4I2MI3MI2M7I", alist->alignments[1]->alignment);
        ck_assert_int_eq(1146, alist->alignments[2]->db_seq.ID);
        ck_assert_int_eq(92, alist->alignments[2]->score);
        ck_assert_str_eq("5M2I5M6I3M4I5M13I2M10I2M3I6M5I4M8I7MI5MI4M2I3MI3M10I", alist->alignments[2]->alignment);
        ck_assert_int_eq(612, alist->alignments[3]->db_seq.ID);
        ck_assert_int_eq(92, alist->alignments[3]->score);
        ck_assert_str_eq("5M2I5M3I7M3I2MI2M2I3M14IM10I3M5I4M8I7MI5MI3M4I2MI3MI2M7I", alist->alignments[3]->alignment);
        ck_assert_int_eq(75, alist->alignments[4]->db_seq.ID);
        ck_assert_int_eq(92, alist->alignments[4]->score);
        ck_assert_str_eq("5M2I5M3I8M7IM8IM8I2M3I6M5I4M3IMIMI4M3IMI5MI3M4I2MI3MI2M7I", alist->alignments[4]->alignment);

        free_alignment(alist);

        free_db();
        free_sequence(query);

        ssa_exit();
    }END_TEST

START_TEST (test_sw_multiple_threads)
    {
        init_score_matrix(BLOSUM62);
        init_gap_penalties(4, 2);
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);

        init_db_fasta("tests/testdata/AF091148.fas");

        p_query query = init_sequence_fasta("tests/testdata/one_seq.fas");

        p_alignment_list alist = sw_align(query, 5);
        ck_assert_int_eq(5, alist->len);

        ck_assert_int_eq(1290, alist->alignments[0]->db_seq.ID);
        ck_assert_int_eq(212, alist->alignments[0]->score);
        ck_assert_str_eq("7M2I3MD3MD3M2D9M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[0]->alignment);
        ck_assert_int_eq(1075, alist->alignments[1]->db_seq.ID);
        ck_assert_int_eq(212, alist->alignments[1]->score);
        ck_assert_str_eq("7M2I3MD3MD3M2D9M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[1]->alignment);
        ck_assert_int_eq(1337, alist->alignments[2]->db_seq.ID);
        ck_assert_int_eq(210, alist->alignments[2]->score);
        ck_assert_str_eq("7M2I3MD3MD4M2D8M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[2]->alignment);
        ck_assert_int_eq(1164, alist->alignments[3]->db_seq.ID);
        ck_assert_int_eq(210, alist->alignments[3]->score);
        ck_assert_str_eq("7M2I3MD3MD4M2D8M5I4M8I7MI5MI3M4I2MI2MI2M", alist->alignments[3]->alignment);
        ck_assert_int_eq(1146, alist->alignments[4]->db_seq.ID);
        ck_assert_int_eq(210, alist->alignments[4]->score);
        ck_assert_str_eq("7M2I3MD3MD4M2D8M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[4]->alignment);


        free_alignment(alist);

        free_db();
        free_sequence(query);

        ssa_exit();
    }END_TEST

START_TEST (test_nw_multiple_threads)
    {
        init_score_matrix(BLOSUM62);
        init_gap_penalties(4, 2);
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);

        init_db_fasta("tests/testdata/AF091148.fas");

        p_query query = init_sequence_fasta("tests/testdata/one_seq.fas");

        p_alignment_list alist = nw_align(query, 5);
        ck_assert_int_eq(5, alist->len);

        ck_assert_int_eq(1050, alist->alignments[0]->db_seq.ID);
        ck_assert_int_eq(112, alist->alignments[0]->score);
        ck_assert_str_eq("5M2I5M3I8M3I2M15I4M3IMI3M5I4M8I7MI5MI3M4I2MI3MI2M7I", alist->alignments[0]->alignment);
        ck_assert_int_eq(378, alist->alignments[1]->db_seq.ID);
        ck_assert_int_eq(100, alist->alignments[1]->score);
        ck_assert_str_eq("5M2I5M3I8M7IM8IM8I2M3I6M5I4M8I7MI5MI3M4I2MI3MI2M7I", alist->alignments[1]->alignment);
        ck_assert_int_eq(1146, alist->alignments[2]->db_seq.ID);
        ck_assert_int_eq(92, alist->alignments[2]->score);
        ck_assert_str_eq("5M2I5M6I3M4I5M13I2M10I2M3I6M5I4M8I7MI5MI4M2I3MI3M10I", alist->alignments[2]->alignment);
        ck_assert_int_eq(612, alist->alignments[3]->db_seq.ID);
        ck_assert_int_eq(92, alist->alignments[3]->score);
        ck_assert_str_eq("5M2I5M3I7M3I2MI2M2I3M14IM10I3M5I4M8I7MI5MI3M4I2MI3MI2M7I", alist->alignments[3]->alignment);
        ck_assert_int_eq(75, alist->alignments[4]->db_seq.ID);
        ck_assert_int_eq(92, alist->alignments[4]->score);
        ck_assert_str_eq("5M2I5M3I8M7IM8IM8I2M3I6M5I4M3IMIMI4M3IMI5MI3M4I2MI3MI2M7I", alist->alignments[4]->alignment);

        free_alignment(alist);

        free_db();
        free_sequence(query);

        ssa_exit();
    }END_TEST

START_TEST (test_init_functions)
    {
        init_score_matrix(BLOSUM45);
        init_score_matrix(BLOSUM50);
        init_score_matrix(BLOSUM62);
        init_score_matrix(BLOSUM80);
        init_score_matrix(BLOSUM90);
        init_score_matrix(PAM30);
        init_score_matrix(PAM70);
        init_score_matrix(PAM250);
        free_matrix();

        init_score_matrix_file("tests/testdata/blosum90.txt");
        free_matrix();

        init_score_matrix_string(mat_blosum80);
        free_matrix();

        init_gap_penalties(8, 5);
        ck_assert_int_eq(8, gapO);
        ck_assert_int_eq(5, gapE);

        init_scoring(3, 6);
        free_matrix();

        init_symbol_translation(NUCLEOTIDE, BOTH_STRANDS, 3, 3);
        ck_assert_int_eq(NUCLEOTIDE, symtype);
        ck_assert_int_eq(BOTH_STRANDS, query_strands);

        init_db_fasta("tests/testdata/AF091148.fas");
        ck_assert_int_eq(1403, ssa_db_get_sequence_count());
        free_db();

        p_query query = init_sequence_fasta("tests/testdata/one_seq.fas");
        ck_assert_str_eq("97485665bcded44c4d86c131ca714848", query->header);
        free_sequence(query);

        ssa_exit();
    }END_TEST

void addLibssaTC(Suite *s) {
    TCase *tc_core = tcase_create("libssa");
    tcase_add_test(tc_core, test_simple_one_thread);
    tcase_add_test(tc_core, test_sw_multiple_threads);
    tcase_add_test(tc_core, test_nw_multiple_threads);
    tcase_add_test(tc_core, test_init_functions);

    suite_add_tcase(s, tc_core);
}