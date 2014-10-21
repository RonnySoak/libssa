/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/algo/aligner.h"
#include "../../src/algo/align.h"
#include "../../src/algo/manager.h"

#include "../../src/util.h"
#include "../../src/libssa.h"
#include "../../src/util/minheap.h"
#include "../../src/matrices.h"
#include "../../src/db_iterator.h"
#include "../../src/query.h"

extern p_alignment_data adp;

elem_t new_elem0(p_sdb_sequence sdb, int qid, long score) {
    elem_t e;
    e.db_id = sdb->ID;
    e.dframe = sdb->frame;
    e.dstrand = sdb->strand;
    e.query_id = qid;
    e.score = score;

    return e;
}

START_TEST (test_aligner_simple_sw)
    {
        mat_init_constant_scoring(1, -1);
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);

        p_query query = query_read("./tests/testdata/short_query.fas");
        ssa_db_init_fasta("./tests/testdata/short_db.fas");

        it_init(1);

        p_minheap heap = minheap_init(1);

        p_sdb_sequence sdb = it_next_sequence();

        elem_t e1 = new_elem0(sdb, 0, 2);
        elem_t * elements = { &e1 };

        init_for_sw(query, 1);
        adp->pair_count = 1;
        adp->result_sequence_pairs = elements;
        set_alignment_data(adp);

        p_alignment_list alist = a_align(NULL);

        ck_assert_int_eq(1, alist->len);

        alignment_p al = alist->alignments[0];

        ck_assert_str_eq(sdb->seq.seq, al->db_seq.seq);
        ck_assert_int_eq(sdb->seq.len, al->db_seq.len);
        ck_assert_int_eq(sdb->frame, al->db_seq.frame);
        ck_assert_int_eq(sdb->strand, al->db_seq.strand);
        ck_assert_int_eq(sdb->ID, al->db_seq.ID);

        ck_assert_str_eq(query->nt[0].seq, al->query.seq);
        ck_assert_int_eq(query->nt[0].len, al->query.len);
        ck_assert_int_eq(0, al->query.frame);
        ck_assert_int_eq(0, al->query.strand);

        ck_assert_int_eq(2, al->score); // TODO check this
        ck_assert_int_eq(1, al->align_d_start);
        ck_assert_int_eq(2, al->align_d_end);
        ck_assert_int_eq(0, al->align_q_start);
        ck_assert_int_eq(1, al->align_q_end);

        ck_assert_str_eq("2M", al->alignment); // TODO check this

        it_free_sequence(sdb);

        a_free(alist);
        minheap_exit(heap);

        mat_free();
        query_free(query);
        it_free();
        ssa_db_free();
    }END_TEST

START_TEST (test_aligner_more_sequences_sw)
    {
        mat_init_constant_scoring(1, -1);
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);

        p_query query = query_read("./tests/testdata/one_seq.fas");
        ssa_db_init_fasta("./tests/testdata/test.fas");

        it_init(3);

        p_minheap heap = minheap_init(5);

        p_sdb_sequence sdb0 = it_next_sequence();

        elem_t e = new_elem0(sdb0, 0, 2);
        minheap_add(heap, &e);

        p_sdb_sequence sdb = it_next_sequence();
        e = new_elem0(sdb0, 0, 2);
        minheap_add(heap, &e);
        it_free_sequence(sdb);

        sdb = it_next_sequence();
        e = new_elem0(sdb0, 0, 2);
        minheap_add(heap, &e);
        it_free_sequence(sdb);

        sdb = it_next_sequence();
        e = new_elem0(sdb0, 0, 2);
        minheap_add(heap, &e);
        it_free_sequence(sdb);

        sdb = it_next_sequence();
        e = new_elem0(sdb0, 0, 2);
        minheap_add(heap, &e);
        it_free_sequence(sdb);

        init_for_sw(query, 1);
        adp->pair_count = 5;
        adp->result_sequence_pairs = heap->array;
        set_alignment_data(adp);

        p_alignment_list alist = a_align(NULL);

        ck_assert_int_eq(5, alist->len);

        alignment_p al = alist->alignments[0];

        ck_assert_str_eq(sdb0->seq.seq, al->db_seq.seq);
        ck_assert_int_eq(sdb0->seq.len, al->db_seq.len);
        ck_assert_int_eq(sdb0->frame, al->db_seq.frame);
        ck_assert_int_eq(sdb0->strand, al->db_seq.strand);
        ck_assert_int_eq(sdb0->ID, al->db_seq.ID);

        ck_assert_str_eq(query->nt[0].seq, al->query.seq);
        ck_assert_int_eq(query->nt[0].len, al->query.len);
        ck_assert_int_eq(0, al->query.frame);
        ck_assert_int_eq(0, al->query.strand);

        ck_assert_int_eq(2, al->score); // score 2, since we set it to 2
        ck_assert_int_eq(1, al->align_d_start);
        ck_assert_int_eq(120, al->align_d_end);
        ck_assert_int_eq(1, al->align_q_start);
        ck_assert_int_eq(53, al->align_q_end);
        ck_assert_str_eq(
                "DMI2MI2MIM3IM2IM4IM9I2MIM4I2M4I2M7IM6IMI3M" \
                "3I2MI2M6ID3MIM3IM3IMIMD4M4IMI4MDMIMIMI3MDMI",
                al->alignment); // TODO check this

        it_free_sequence(sdb0);

        a_free(alist);
        minheap_exit(heap);

        it_free();
        mat_free();
        query_free(query);
        ssa_db_free();
    }END_TEST

void addAlignerTC(Suite *s) {
    TCase *tc_core = tcase_create("aligner");
    tcase_add_test(tc_core, test_aligner_simple_sw);
    tcase_add_test(tc_core, test_aligner_more_sequences_sw);

    suite_add_tcase(s, tc_core);
}
