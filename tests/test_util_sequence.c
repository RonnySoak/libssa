/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "tests.h"
#include "../src/util/util.h"
#include "../src/util/util_sequence.h"

START_TEST (test_revcompl)
    {
        // test always the same
        sequence seq;
        seq.seq = xmalloc( 5 );
        seq.len = 4;
        seq.seq[0] = 1;
        seq.seq[1] = 2;
        seq.seq[2] = 3;
        seq.seq[3] = 4;
        seq.seq[4] = 0;

        sequence rc1 = { xmalloc( seq.len + 1 ), seq.len };
        us_revcompl( seq, rc1 );
        sequence rc2 = { xmalloc( seq.len + 1 ), seq.len };
        us_revcompl( seq, rc2 );
        ck_assert_str_eq( rc1.seq, rc2.seq );
        free( rc1.seq );
        free( rc2.seq );

        // test rev compl calculation
        char ntcompl[16] = { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };

        char rev_seq[seq.len];

        rev_seq[0] = ntcompl[(int) (seq.seq[3])];
        rev_seq[1] = ntcompl[(int) (seq.seq[2])];
        rev_seq[2] = ntcompl[(int) (seq.seq[1])];
        rev_seq[3] = ntcompl[(int) (seq.seq[0])];

        rc2 = (sequence ) { xmalloc( seq.len + 1 ), seq.len };
        us_revcompl( seq, rc2 );
        ck_assert_str_eq( rev_seq, rc2.seq );
        free( rc2.seq );

        free( seq.seq );
    }END_TEST

START_TEST (test_map_sequence)
    {
        sequence orig = { xmalloc( 5 ), 4 };
        orig.seq[0] = 'A';
        orig.seq[1] = 'C';
        orig.seq[2] = 'A';
        orig.seq[3] = 'T';
        orig.seq[4] = 0;

        sequence mapped = { xmalloc( 5 ), 4 };

        us_map_sequence( orig, mapped, map_ncbi_nt16 );

        ck_assert_int_eq( 4, mapped.len );
        ck_assert_int_eq( 1, mapped.seq[0] );
        ck_assert_int_eq( 2, mapped.seq[1] );
        ck_assert_int_eq( 1, mapped.seq[2] );
        ck_assert_int_eq( 8, mapped.seq[3] );
        ck_assert_int_eq( 0, mapped.seq[4] );
    }END_TEST

START_TEST (test_map_sequence_illegal_symbol)
    {
        sequence orig = { xmalloc( 5 ), 4 };
        orig.seq[0] = 'A';
        orig.seq[1] = 'X';
        orig.seq[2] = 'A';
        orig.seq[3] = '2';
        orig.seq[4] = 0;

        sequence mapped = { xmalloc( 5 ), 4 };

        us_map_sequence( orig, mapped, map_ncbi_nt16 );

        ck_assert_int_eq( 4, mapped.len );
        ck_assert_int_eq( 1, mapped.seq[0] );
        ck_assert_int_eq( 0, mapped.seq[1] );
        ck_assert_int_eq( 1, mapped.seq[2] );
        ck_assert_int_eq( 0, mapped.seq[3] );
        ck_assert_int_eq( 0, mapped.seq[4] );
    }END_TEST

void ck_converted_prot_eq( char* ref, sequence seq ) {
    sequence conv_dna;
    conv_dna.len = seq.len;
    conv_dna.seq = xmalloc( seq.len + 1 );
    for( size_t i = 0; i < seq.len; i++ ) {
        conv_dna.seq[i] = sym_ncbi_aa[(int) seq.seq[i]];
    }
    conv_dna.seq[seq.len] = 0;

    ck_assert_str_eq( ref, conv_dna.seq );
}

START_TEST (test_translate_query_RNA)
    {
        us_init_translation( 3, 1 );

        char* dna = "AUGCCCAAGCUGAAUAGCGUAGAGGGGUUUUCAUCAUUUGAGGACGAUGUAUAA";
        size_t dlen = strlen( dna );
        sequence protp = { xmalloc( 1 ), 0 };

        sequence conv_dna;
        conv_dna.len = dlen;
        conv_dna.seq = xmalloc( dlen + 1 );
        for( size_t i = 0; i < dlen; i++ ) {
            conv_dna.seq[i] = map_ncbi_nt16[(int) dna[i]];
        }
        conv_dna.seq[dlen] = 0;
        us_translate_sequence( 0, conv_dna, 0, 0, &protp );

        ck_assert_int_eq( 18, protp.len );

        ck_converted_prot_eq( "MPKTNSVEGFSSFEDDV*", protp );
        free( protp.seq );
        free( conv_dna.seq );
    }END_TEST

START_TEST (test_translate_query)
    {
        us_init_translation( 3, 1 );

        char* dna = "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA";
        size_t dlen = strlen( dna );
        sequence protp = { xmalloc( 1 ), 0 };

        sequence conv_dna;
        conv_dna.len = dlen;
        conv_dna.seq = xmalloc( dlen + 1 );
        for( size_t i = 0; i < dlen; i++ ) {
            conv_dna.seq[i] = map_ncbi_nt16[(int) dna[i]];
        }
        conv_dna.seq[dlen] = 0;

        // forward strand
        // frame 1
        us_translate_sequence( 0, conv_dna, 0, 0, &protp );

        ck_assert_int_eq( 18, protp.len );
        ck_converted_prot_eq( "MPKTNSVEGFSSFEDDV*", protp );

        // frame 2
        us_translate_sequence( 0, conv_dna, 0, 1, &protp );

        ck_assert_int_eq( 17, protp.len );
        ck_converted_prot_eq( "CPSWMA*RGFHHLRTMY", protp );

        // frame 3
        us_translate_sequence( 0, conv_dna, 0, 2, &protp );
        ck_assert_int_eq( 17, protp.len );
        ck_converted_prot_eq( "AQAE*RRGVFIIWGRCM", protp );

        // complementary strand
        // frame 1
        us_translate_sequence( 0, conv_dna, 1, 0, &protp );

        ck_assert_int_eq( 18, protp.len );
        ck_converted_prot_eq( "LYIVTKWWKPTYAIQTGH", protp );

        // frame 2
        us_translate_sequence( 0, conv_dna, 1, 1, &protp );

        ck_assert_int_eq( 17, protp.len );
        ck_converted_prot_eq( "YTSSSNDENPSTTFSLG", protp );

        // frame 3
        us_translate_sequence( 0, conv_dna, 1, 2, &protp );
        ck_assert_int_eq( 17, protp.len );
        ck_converted_prot_eq( "MHRPQMMKTPTRYSAWA", protp );

        free( protp.seq );
        free( conv_dna.seq );
    }END_TEST

START_TEST (test_translate_query_DNA)
    {
        us_init_translation( 3, 1 );

        char* dna = "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA";
        size_t dlen = strlen( dna );
        sequence protp = { xmalloc( 1 ), 0 };

        sequence conv_dna;
        conv_dna.len = dlen;
        conv_dna.seq = xmalloc( dlen + 1 );
        for( size_t i = 0; i < dlen; i++ ) {
            conv_dna.seq[i] = map_ncbi_nt16[(int) dna[i]];
        }
        conv_dna.seq[dlen] = 0;

        us_translate_sequence( 0, conv_dna, 0, 0, &protp );

        ck_assert_int_eq( 18, protp.len );
        ck_converted_prot_eq( "MPKTNSVEGFSSFEDDV*", protp );
        free( protp.seq );
        free( conv_dna.seq );
    }END_TEST

START_TEST (test_translate_db)
    {
        us_init_translation( 1, 3 );

        char* dna = "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA";
        size_t dlen = strlen( dna );
        sequence protp = { xmalloc( 1 ), 0 };

        sequence conv_dna;
        conv_dna.len = dlen;
        conv_dna.seq = xmalloc( dlen + 1 );
        for( size_t i = 0; i < dlen; i++ ) {
            conv_dna.seq[i] = map_ncbi_nt16[(int) dna[i]];
        }
        conv_dna.seq[dlen] = 0;

        // translate it as a db sequence, using the db translation table
        us_translate_sequence( 1, conv_dna, 0, 0, &protp );

        ck_assert_int_eq( 18, protp.len );
        ck_converted_prot_eq( "MPKTNSVEGFSSFEDDV*", protp );
        free( protp.seq );
        free( conv_dna.seq );
    }END_TEST

void addUtilSequenceTC( Suite *s ) {
    TCase *tc_core = tcase_create( "util_sequence" );
    tcase_add_test( tc_core, test_revcompl );
    tcase_add_test( tc_core, test_map_sequence );
    tcase_add_test( tc_core, test_map_sequence_illegal_symbol );
    tcase_add_test( tc_core, test_translate_query );
    tcase_add_test( tc_core, test_translate_query_DNA );
    tcase_add_test( tc_core, test_translate_query_RNA );
    tcase_add_test( tc_core, test_translate_db );

    suite_add_tcase( s, tc_core );
}

