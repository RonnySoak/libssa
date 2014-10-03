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

extern p_alignment_list a_align(p_minheap heap);

START_TEST (test_aligner_simple)
    {
        mat_init_constant_scoring(1, -1);
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);

        p_query query = query_read("./tests/testdata/short_query.fas");
        ssa_db_init_fasta("./tests/testdata/short_db.fas");

        it_init(1);

        p_minheap heap = minheap_init(1);

        p_sdb_sequence sdb = it_next_sequence();

        elem_t e;
        e.db_seq = sdb;
        e.query = query;
        e.score = 2;
        minheap_add(heap, &e);

        p_alignment_list alignment_list = a_align(heap);

        ck_assert_int_eq(1, alignment_list->len);

        p_alignment al = alignment_list->alignments[0];

        ck_assert_str_eq(sdb->seq.seq, al->db_seq.seq);
        ck_assert_int_eq(sdb->seq.len, al->db_seq.len);
        ck_assert_int_eq(sdb->frame, al->db_seq.frame);
        ck_assert_int_eq(sdb->strand, al->db_seq.strand);
        ck_assert_str_eq(sdb->info->header, al->db_seq.header);
        ck_assert_int_eq(sdb->info->headerlen, al->db_seq.headerlen);
        ck_assert_int_eq(sdb->info->ID, al->db_seq.ID);

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

        p_sdb_sequence sdb = it_next_sequence();

        elem_t e;
        e.db_seq = sdb;
        e.query = query;
        e.score = 2;
        minheap_add(heap, &e);
        p_sdb_sequence sdb2 = it_next_sequence();
        elem_t e2;
        e2.db_seq = sdb2;
        e2.query = query;
        e2.score = 2;
        minheap_add(heap, &e2);
        p_sdb_sequence sdb3 = it_next_sequence();
        elem_t e3;
        e3.db_seq = sdb3;
        e3.query = query;
        e3.score = 2;
        minheap_add(heap, &e3);
        p_sdb_sequence sdb4 = it_next_sequence();
        elem_t e4;
        e4.db_seq = sdb4;
        e4.query = query;
        e4.score = 2;
        minheap_add(heap, &e4);
        p_sdb_sequence sdb5 = it_next_sequence();
        elem_t e5;
        e5.db_seq = sdb5;
        e5.query = query;
        e5.score = 2;
        minheap_add(heap, &e5);

        p_alignment_list alignment_list = a_align(heap);

        ck_assert_int_eq(5, alignment_list->len);

        p_alignment al = alignment_list->alignments[0];

        ck_assert_str_eq(sdb->seq.seq, al->db_seq.seq);
        ck_assert_int_eq(sdb->seq.len, al->db_seq.len);
        ck_assert_int_eq(sdb->frame, al->db_seq.frame);
        ck_assert_int_eq(sdb->strand, al->db_seq.strand);
        ck_assert_str_eq(sdb->info->header, al->db_seq.header);
        ck_assert_int_eq(sdb->info->headerlen, al->db_seq.headerlen);
        ck_assert_int_eq(sdb->info->ID, al->db_seq.ID);

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

        mat_free();
        query_free(query);
        it_free();
        ssa_db_free();
    }END_TEST

void addAlignerTC(Suite *s) {
    TCase *tc_core = tcase_create("aligner");
    tcase_add_test(tc_core, test_aligner_simple);
    tcase_add_test(tc_core, test_aligner_more_sequences);

    suite_add_tcase(s, tc_core);
}
