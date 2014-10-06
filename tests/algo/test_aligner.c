/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/util.h"
#include "../../src/libssa.h"
#include "../../src/util/minheap.h"

extern void mat_init_constant_scoring(const uint8_t matchscore,
        const uint8_t mismatchscore);
extern void mat_free();

extern p_query query_read(char* filename);
extern void query_free(p_query p);

extern void it_init(long chunk_count);
extern p_sdb_sequence it_next_sequence();
extern void it_free();
extern void it_free_sequence(p_sdb_sequence seq);

extern p_alignment_list a_align(p_minheap heap, seq_buffer* queries, int q_count);
extern void a_free(p_alignment_list alist);

elem_t new_elem0(p_sdb_sequence sdb, int qid, long score) {
    elem_t e;
    e.db_id = sdb->ID;
    e.dframe = sdb->frame;
    e.dstrand = sdb->strand;
    e.query_id = qid;
    e.score = score;

    return e;
}

START_TEST (test_aligner_simple)
    {
        mat_init_constant_scoring(1, -1);
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);

        p_query query = query_read("./tests/testdata/short_query.fas");
        ssa_db_init_fasta("./tests/testdata/short_db.fas");

        it_init(1);

        p_minheap heap = minheap_init(1);

        p_sdb_sequence sdb = it_next_sequence();

        elem_t e = new_elem0(sdb, 0, 2);
        minheap_add(heap, &e);

        seq_buffer queries[1];
        queries[0].seq = query->nt[0];
        queries[0].strand = 0;
        queries[0].frame = 0;

        p_alignment_list alist = a_align(heap, queries, 0);

        ck_assert_int_eq(1, alist->len);

        p_alignment al = alist->alignments[0];

        ck_assert_str_eq(sdb->seq.seq, al->db_seq.seq);
        ck_assert_int_eq(sdb->seq.len, al->db_seq.len);
        ck_assert_int_eq(sdb->frame, al->db_seq.frame);
        ck_assert_int_eq(sdb->strand, al->db_seq.strand);
        ck_assert_int_eq(sdb->ID, al->db_seq.ID);

        ck_assert_str_eq(query->nt[0].seq, al->query.seq);
        ck_assert_int_eq(query->nt[0].len, al->query.len);
        ck_assert_int_eq(0, al->query.frame);
        ck_assert_int_eq(0, al->query.strand);

        ck_assert_int_eq(0, al->score); // TODO check this
        ck_assert_int_eq(2, al->score_align);
        ck_assert_int_eq(1, al->align_d_start);
        ck_assert_int_eq(2, al->align_d_end);
        ck_assert_int_eq(0, al->align_q_start);
        ck_assert_int_eq(1, al->align_q_end);

        ck_assert_str_eq("M2", al->alignment); // TODO check this

        it_free_sequence(sdb);

        a_free(alist);
        minheap_exit(heap);

        mat_free();
        query_free(query);
        it_free();
        ssa_db_free();
    }END_TEST

START_TEST (test_aligner_more_sequences)
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

        seq_buffer queries[1];
        queries[0].seq = query->nt[0];
        queries[0].strand = 0;
        queries[0].frame = 0;

        p_alignment_list alist = a_align(heap, queries, 0);

        ck_assert_int_eq(5, alist->len);

        p_alignment al = alist->alignments[0];

        ck_assert_str_eq(sdb0->seq.seq, al->db_seq.seq);
        ck_assert_int_eq(sdb0->seq.len, al->db_seq.len);
        ck_assert_int_eq(sdb0->frame, al->db_seq.frame);
        ck_assert_int_eq(sdb0->strand, al->db_seq.strand);
        ck_assert_int_eq(sdb0->ID, al->db_seq.ID);

        ck_assert_str_eq(query->nt[0].seq, al->query.seq);
        ck_assert_int_eq(query->nt[0].len, al->query.len);
        ck_assert_int_eq(0, al->query.frame);
        ck_assert_int_eq(0, al->query.strand);

        ck_assert_int_eq(0, al->score); // TODO check this
        ck_assert_int_eq(49, al->score_align);
        ck_assert_int_eq(1, al->align_d_start);
        ck_assert_int_eq(120, al->align_d_end);
        ck_assert_int_eq(1, al->align_q_start);
        ck_assert_int_eq(53, al->align_q_end);

        ck_assert_str_eq(
                "M1I1M2I1M2I1M1I3M1I2M1I4M1I1M1I1M1I3M1I3M1I1M1I3M1I4M1D1I1M1I1M2I11"\
                "M2I2M1I4M1I1M3I5M3I5M1I1M1I2M1D1I1M4I1M5D1I1M1I1M1I4M1I1M3D1M1I1M1",
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
    tcase_add_test(tc_core, test_aligner_simple);
    tcase_add_test(tc_core, test_aligner_more_sequences);

    suite_add_tcase(s, tc_core);
}
