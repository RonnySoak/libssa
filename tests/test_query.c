/*
 * test_query.c
 *
 *  Created on: 03 Sep 2014
 *      Author: Jakob Frielingsdorf
 */

#include "tests.h"

#include "../src/libssa_datatypes.h"
#include "../src/util.h"

extern char * revcompl(char * seq, long len);
extern void query_init(char * queryname, long strands);
extern int query_read();
extern void query_exit();

extern int symtype;

extern struct _query query;
extern const char * sym_ncbi_nt4;
extern const char * sym_ncbi_nt16;
extern const char * sym_ncbi_nt16u;
extern const char * sym_ncbi_aa;

extern char map_ncbi_nt4[256];
extern char map_ncbi_nt16[256];
extern char map_ncbi_aa[256];

START_TEST (test_query_read_sym0)
    {
        int strands = 2;
        symtype = 0;

        query_init("./tests/testdata/NP_009305.1.fas", strands);
        ck_assert_int_eq(1, query_read());

        ck_assert_msg(NULL != query.nt[0].seq); // TODO map sequence in NT back and compare them to the original
        ck_assert_msg(NULL != query.nt[1].seq);

        // as defined for strands == 2
        ck_assert_str_eq(query.nt[1].seq,
                revcompl(query.nt[0].seq, query.nt[0].len));

        ck_assert_msg(NULL == query.aa[0].seq);
        ck_assert_msg(NULL == query.aa[1].seq);
        ck_assert_msg(NULL == query.aa[2].seq);
        ck_assert_msg(NULL == query.aa[3].seq);
        ck_assert_msg(NULL == query.aa[4].seq);
        ck_assert_msg(NULL == query.aa[5].seq);

        ck_assert_int_eq(65, (int )query.dlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query.description);

        ck_assert_str_eq(sym_ncbi_nt16, query.sym);
        ck_assert_str_eq(map_ncbi_nt16, query.map);
        ck_assert_int_eq(strands, (int )query.strands);

        query_exit();
    }END_TEST

START_TEST (test_query_read_sym1)
    {
        int strands = 2;
        symtype = 1;

        query_init("./tests/testdata/NP_009305.1.fas", strands);
        ck_assert_int_eq(1, query_read());

        ck_assert_msg(NULL == query.nt[0].seq);
        ck_assert_msg(NULL == query.nt[1].seq);

        ck_assert_msg(NULL != query.aa[0].seq); // TODO map sequence in NT back and compare them to the original
        ck_assert_msg(NULL == query.aa[1].seq);
        ck_assert_msg(NULL == query.aa[2].seq);
        ck_assert_msg(NULL == query.aa[3].seq);
        ck_assert_msg(NULL == query.aa[4].seq);
        ck_assert_msg(NULL == query.aa[5].seq);

        ck_assert_int_eq(65, (int )query.dlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query.description);

        ck_assert_str_eq(sym_ncbi_aa, query.sym);
        ck_assert_str_eq(map_ncbi_aa, query.map);
        ck_assert_int_eq(strands, (int )query.strands);

        query_exit();
    }END_TEST

START_TEST (test_query_read_sym2)
    {
        int strands = 2;
        symtype = 2;

        query_init("./tests/testdata/NP_009305.1.fas", strands);
        ck_assert_int_eq(1, query_read());

        ck_assert_msg(NULL != query.nt[0].seq); // TODO map sequence in NT back and compare them to the original
        ck_assert_msg(NULL != query.nt[1].seq);

        // as defined for strands == 2
        ck_assert_str_eq(query.nt[1].seq,
                revcompl(query.nt[0].seq, query.nt[0].len));

        ck_assert_msg(NULL == query.aa[0].seq);
        ck_assert_msg(NULL == query.aa[1].seq);
        ck_assert_msg(NULL == query.aa[2].seq);
        ck_assert_msg(NULL != query.aa[3].seq);
        ck_assert_msg(NULL != query.aa[4].seq);
        ck_assert_msg(NULL != query.aa[5].seq);

        ck_assert_int_eq(65, (int )query.dlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query.description);

        ck_assert_str_eq(sym_ncbi_nt16, query.sym);
        ck_assert_str_eq(map_ncbi_nt16, query.map);
        ck_assert_int_eq(strands, (int )query.strands);

        query_exit();
    }END_TEST

START_TEST (test_query_read_sym3)
    {
        int strands = 2;
        symtype = 3;

        query_init("./tests/testdata/NP_009305.1.fas", strands);
        ck_assert_int_eq(1, query_read());

        ck_assert_msg(NULL == query.nt[0].seq); // TODO map sequence in NT back and compare them to the original
        ck_assert_msg(NULL == query.nt[1].seq);

        ck_assert_msg(NULL != query.aa[0].seq);
        ck_assert_msg(NULL == query.aa[1].seq);
        ck_assert_msg(NULL == query.aa[2].seq);
        ck_assert_msg(NULL == query.aa[3].seq);
        ck_assert_msg(NULL == query.aa[4].seq);
        ck_assert_msg(NULL == query.aa[5].seq);

        ck_assert_int_eq(65, (int )query.dlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query.description);

        ck_assert_str_eq(sym_ncbi_aa, query.sym);
        ck_assert_str_eq(map_ncbi_aa, query.map);
        ck_assert_int_eq(strands, (int )query.strands);

        query_exit();
    }END_TEST

START_TEST (test_query_read_sym4)
    {
        int strands = 2;
        symtype = 4;

        query_init("./tests/testdata/NP_009305.1.fas", strands);
        ck_assert_int_eq(1, query_read());

        ck_assert_msg(NULL != query.nt[0].seq); // TODO map sequence in NT back and compare them to the original
        ck_assert_msg(NULL != query.nt[1].seq);

        // as defined for strands == 2
        ck_assert_str_eq(query.nt[1].seq,
                revcompl(query.nt[0].seq, query.nt[0].len));

        ck_assert_msg(NULL == query.aa[0].seq);
        ck_assert_msg(NULL == query.aa[1].seq);
        ck_assert_msg(NULL == query.aa[2].seq);
        ck_assert_msg(NULL != query.aa[3].seq);
        ck_assert_msg(NULL != query.aa[4].seq);
        ck_assert_msg(NULL != query.aa[5].seq);

        ck_assert_int_eq(65, (int )query.dlen);
        ck_assert_str_eq(
                "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p",
                query.description);

        ck_assert_str_eq(sym_ncbi_nt16, query.sym);
        ck_assert_str_eq(map_ncbi_nt16, query.map);
        ck_assert_int_eq(strands, (int )query.strands);

        query_exit();
    }END_TEST

void addQueryTC(Suite *s) {
    init_out(NULL);

    TCase *tc_core = tcase_create("query");
    tcase_add_test(tc_core, test_query_read_sym0);
    tcase_add_test(tc_core, test_query_read_sym1);
    tcase_add_test(tc_core, test_query_read_sym2);
    tcase_add_test(tc_core, test_query_read_sym3);
    tcase_add_test(tc_core, test_query_read_sym4);

    suite_add_tcase(s, tc_core);
}
