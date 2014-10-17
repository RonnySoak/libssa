/*
 * test_query->c
 *
 *  Created on: 03 Sep 2014
 *      Author: Jakob Frielingsdorf
 */

#include "tests.h"

#include "../src/libssa.h"
#include "../src/libssa_datatypes.h"
#include "../src/util.h"
#include "../src/query.h"
#include "../src/util_sequence.h"

START_TEST (test_query_read_sym0)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = NUCLEOTIDE;

        p_query query = query_read("./tests/testdata/NP_009305.1.fas");
        ck_assert_msg(NULL != query->nt[0].seq);
        ck_assert_msg(NULL != query->nt[1].seq);

        // as defined for strands == 2
        ck_assert_str_eq(query->nt[1].seq, us_revcompl(query->nt[0]).seq);

        ck_assert_msg(NULL == query->aa[0].seq);
        ck_assert_msg(NULL == query->aa[1].seq);
        ck_assert_msg(NULL == query->aa[2].seq);
        ck_assert_msg(NULL == query->aa[3].seq);
        ck_assert_msg(NULL == query->aa[4].seq);
        ck_assert_msg(NULL == query->aa[5].seq);

        ck_assert_int_eq(65, (int )query->headerlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query->header);

        ck_assert_str_eq(sym_ncbi_nt16, query->sym);
        ck_assert_str_eq(map_ncbi_nt16, query->map);

        query_free(query);
    }END_TEST

START_TEST (test_query_read_sym1)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = AMINOACID;

        p_query query = query_read("./tests/testdata/NP_009305.1.fas");

        ck_assert_msg(NULL == query->nt[0].seq);
        ck_assert_msg(NULL == query->nt[1].seq);

        ck_assert_msg(NULL != query->aa[0].seq);
        ck_assert_msg(NULL == query->aa[1].seq);
        ck_assert_msg(NULL == query->aa[2].seq);
        ck_assert_msg(NULL == query->aa[3].seq);
        ck_assert_msg(NULL == query->aa[4].seq);
        ck_assert_msg(NULL == query->aa[5].seq);

        ck_assert_int_eq(65, (int )query->headerlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query->header);

        ck_assert_str_eq(sym_ncbi_aa, query->sym);
        ck_assert_str_eq(map_ncbi_aa, query->map);

        query_free(query);
    }END_TEST

START_TEST (test_query_read_sym2)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = TRANS_QUERY;

        us_init_translation(3, 3);

        p_query query = query_read("./tests/testdata/NP_009305.1.fas");

        ck_assert_msg(NULL != query->nt[0].seq);
        ck_assert_msg(NULL != query->nt[1].seq);

        // as defined for strands == 2
        ck_assert_str_eq(query->nt[1].seq, us_revcompl(query->nt[0]).seq);

        ck_assert_msg(NULL == query->aa[0].seq);
        ck_assert_msg(NULL == query->aa[1].seq);
        ck_assert_msg(NULL == query->aa[2].seq);
        ck_assert_msg(NULL != query->aa[3].seq);
        ck_assert_msg(NULL != query->aa[4].seq);
        ck_assert_msg(NULL != query->aa[5].seq);

        ck_assert_int_eq(65, (int )query->headerlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query->header);

        ck_assert_str_eq(sym_ncbi_nt16, query->sym);
        ck_assert_str_eq(map_ncbi_nt16, query->map);

        query_free(query);
    }END_TEST

START_TEST (test_query_read_sym3)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = TRANS_DB;

        p_query query = query_read("./tests/testdata/NP_009305.1.fas");

        ck_assert_msg(NULL == query->nt[0].seq);
        ck_assert_msg(NULL == query->nt[1].seq);

        ck_assert_msg(NULL != query->aa[0].seq);
        ck_assert_msg(NULL == query->aa[1].seq);
        ck_assert_msg(NULL == query->aa[2].seq);
        ck_assert_msg(NULL == query->aa[3].seq);
        ck_assert_msg(NULL == query->aa[4].seq);
        ck_assert_msg(NULL == query->aa[5].seq);

        ck_assert_int_eq(65, (int )query->headerlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query->header);

        ck_assert_str_eq(sym_ncbi_aa, query->sym);
        ck_assert_str_eq(map_ncbi_aa, query->map);

        query_free(query);
    }END_TEST

START_TEST (test_query_read_sym4)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = TRANS_BOTH;

        us_init_translation(3, 3);

        p_query query = query_read("./tests/testdata/NP_009305.1.fas");

        ck_assert_msg(NULL != query->nt[0].seq);
        ck_assert_msg(NULL != query->nt[1].seq);

        // as defined for strands == 2
        ck_assert_str_eq(query->nt[1].seq, us_revcompl(query->nt[0]).seq);

        ck_assert_msg(NULL == query->aa[0].seq);
        ck_assert_msg(NULL == query->aa[1].seq);
        ck_assert_msg(NULL == query->aa[2].seq);
        ck_assert_msg(NULL != query->aa[3].seq);
        ck_assert_msg(NULL != query->aa[4].seq);
        ck_assert_msg(NULL != query->aa[5].seq);

        ck_assert_int_eq(65, (int )query->headerlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query->header);

        ck_assert_str_eq(sym_ncbi_nt16, query->sym);
        ck_assert_str_eq(map_ncbi_nt16, query->map);

        query_free(query);
    }END_TEST

START_TEST (test_strands_param)
    {
        // plus
        symtype = TRANS_BOTH;
        query_strands = FORWARD_STRAND;

        us_init_translation(3, 3);

        p_query query = query_read("./tests/testdata/NP_009305.1.fas");

        ck_assert_msg(NULL != query->nt[0].seq);
        ck_assert_msg(NULL == query->nt[1].seq);

        ck_assert_msg(NULL != query->aa[0].seq);
        ck_assert_msg(NULL != query->aa[1].seq);
        ck_assert_msg(NULL != query->aa[2].seq);
        ck_assert_msg(NULL == query->aa[3].seq);
        ck_assert_msg(NULL == query->aa[4].seq);
        ck_assert_msg(NULL == query->aa[5].seq);

        query_free(query);

        // minus
        query_strands = COMPLEMENTARY_STRAND;

        query = query_read("./tests/testdata/NP_009305.1.fas");

        ck_assert_msg(NULL != query->nt[0].seq);
        ck_assert_msg(NULL != query->nt[1].seq);

        // as defined for strands == 2
        ck_assert_str_eq(query->nt[1].seq, us_revcompl(query->nt[0]).seq);

        ck_assert_msg(NULL == query->aa[0].seq);
        ck_assert_msg(NULL == query->aa[1].seq);
        ck_assert_msg(NULL == query->aa[2].seq);
        ck_assert_msg(NULL != query->aa[3].seq);
        ck_assert_msg(NULL != query->aa[4].seq);
        ck_assert_msg(NULL != query->aa[5].seq);

        query_free(query);

        // both
        query_strands = BOTH_STRANDS;

        query = query_read("./tests/testdata/NP_009305.1.fas");

        ck_assert_msg(NULL != query->nt[0].seq);
        ck_assert_msg(NULL != query->nt[1].seq);

        // as defined for strands == 3
        ck_assert_str_eq(query->nt[1].seq,
                us_revcompl(query->nt[0]).seq);

        ck_assert_msg(NULL != query->aa[0].seq);
        ck_assert_msg(NULL != query->aa[1].seq);
        ck_assert_msg(NULL != query->aa[2].seq);
        ck_assert_msg(NULL != query->aa[3].seq);
        ck_assert_msg(NULL != query->aa[4].seq);
        ck_assert_msg(NULL != query->aa[5].seq);

        query_free(query);
    }END_TEST

void addQueryTC(Suite *s) {
    TCase *tc_core = tcase_create("query");
    tcase_add_test(tc_core, test_query_read_sym0);
    tcase_add_test(tc_core, test_query_read_sym1);
    tcase_add_test(tc_core, test_query_read_sym2);
    tcase_add_test(tc_core, test_query_read_sym3);
    tcase_add_test(tc_core, test_query_read_sym4);
    tcase_add_test(tc_core, test_strands_param);

    suite_add_tcase(s, tc_core);
}
