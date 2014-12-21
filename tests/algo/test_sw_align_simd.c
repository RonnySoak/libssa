/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/util.h"
#include "../../src/libssa.h"
#include "../../src/matrices.h"
#include "../../src/db_iterator.h"
#include "../../src/query.h"
#include "../../src/algo/align_simd.h"

START_TEST (test_sw_align_simd_simple)
    {
        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
        mat_init_constant_scoring(1, -1);

        p_query query = query_read("./tests/testdata/short_query.fas");

        ssa_db_init_fasta("./tests/testdata/short_db.fas");
        it_init(1);
        sequence dseq = it_translate_sequence(ssa_db_get_sequence(0), 0, 0);

        uint8_t gapopen = 1;
        uint8_t gapextend = 1;

        struct s16info_s * s16info = search16_init(1, -1, gapopen, gapopen, gapopen, gapopen, gapopen,
                gapopen, gapextend, gapextend, gapextend, gapextend, gapextend, gapextend);

        search16_qprep(s16info, dseq.seq, dseq.len);

        unsigned int sequences = 1;
        unsigned int seqnos = 0;
        int16_t pscores;
        uint16_t paligned;
        unsigned short pmatches;
        unsigned short pmismatches;
        unsigned short pgaps;
        char * pcigar;

        search16(s16info, sequences, &seqnos, &pscores, &paligned, &pmatches, &pmismatches, &pgaps, &pcigar);

        ck_assert_int_eq(pscores, 6);

        search16_exit(s16info);

        it_free();
        mat_free();
        query_free(query);
    }END_TEST

//START_TEST (test_search63_simple_blosum62)
//    {
//        init_symbol_translation(NUCLEOTIDE, FORWARD_STRAND, 3, 3);
//        mat_init_buildin(BLOSUM62);
//
//        p_query query = query_read("./tests/testdata/short_query.fas");
//
//        ssa_db_init_fasta("./tests/testdata/short_db.fas");
//        it_init(1);
//        sequence dseq = it_translate_sequence(ssa_db_get_sequence(0), 0, 0);
//
//        long *hearray = calloc(sizeof(long), 32 * query->nt[0].len);
//        uint8_t gapopenextend = 1;
//        uint8_t gapextend = 1;
//
//        int score = full_sw(&dseq,
//                &query->nt[0],
//                hearray,
//                score_matrix_63,
//                gapopenextend, gapextend);
//
//        ck_assert_int_eq(score, 12);
//
//        it_free();
//        mat_free();
//        query_free(query);
//    }END_TEST

void addSmithWatermanSIMD16TC(Suite *s) {
    TCase *tc_core = tcase_create("SmithWaterman63");
    tcase_add_test(tc_core, test_sw_align_simd_simple);

    suite_add_tcase(s, tc_core);
}
