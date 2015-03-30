/*
 Copyright (C) 2014-2015 Jakob Frielingsdorf

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Contact: Jakob Frielingsdorf <jfrielingsdorf@gmail.com>
 */

#include "tests.h"

#include "../src/libssa.h"
#include "../src/libssa_datatypes.h"
#include "../src/util/util.h"
#include "../src/query.h"
#include "../src/util/util_sequence.h"

char * get_reverse_complement( sequence_t orig ) {
    sequence_t rc = { xmalloc( orig.len + 1 ), orig.len };
    us_revcompl( orig, rc );
    return rc.seq;
}

static void check_query_mapped( int reversed, const char map[256], size_t length, char * expected, sequence_t seq ) {
    ck_assert_ptr_ne( NULL, seq.seq );
    ck_assert_int_eq( length, seq.len );

    if( reversed ) {
        for( int i = length - 1; i < 0; --i ) {
            ck_assert_int_eq( map[(int )expected[i]], seq.seq[i] );
        }
    }
    else {
        for( int i = 0; i < length; ++i ) {
            ck_assert_int_eq( map[(int )expected[i]], seq.seq[i] );
        }
    }
}

static void ck_sequences_set( p_query query ) {
    if( symtype == NUCLEOTIDE ) {
        if( query_strands == FORWARD_STRAND || query_strands == BOTH_STRANDS ) {
            ck_assert_ptr_ne( NULL, query->nt[0].seq );
        }

        if( query_strands == COMPLEMENTARY_STRAND || query_strands == BOTH_STRANDS ) {
            ck_assert_ptr_ne( NULL, query->nt[1].seq );
        }

        ck_assert_ptr_eq( NULL, query->aa[0].seq );
        ck_assert_ptr_eq( NULL, query->aa[1].seq );
        ck_assert_ptr_eq( NULL, query->aa[2].seq );
        ck_assert_ptr_eq( NULL, query->aa[3].seq );
        ck_assert_ptr_eq( NULL, query->aa[4].seq );
        ck_assert_ptr_eq( NULL, query->aa[5].seq );
    }
    else if( symtype == TRANS_QUERY || symtype == TRANS_BOTH ) {
        ck_assert_ptr_eq( NULL, query->nt[0].seq );
        ck_assert_ptr_eq( NULL, query->nt[1].seq );

        if( query_strands == FORWARD_STRAND || query_strands == BOTH_STRANDS ) {
            ck_assert_ptr_ne( NULL, query->aa[0].seq );
            ck_assert_ptr_ne( NULL, query->aa[1].seq );
            ck_assert_ptr_ne( NULL, query->aa[2].seq );

        }

        if( query_strands == COMPLEMENTARY_STRAND || query_strands == BOTH_STRANDS ) {
            ck_assert_ptr_ne( NULL, query->aa[3].seq );
            ck_assert_ptr_ne( NULL, query->aa[4].seq );
            ck_assert_ptr_ne( NULL, query->aa[5].seq );
        }
    }
    else {
        ck_assert_ptr_eq( NULL, query->nt[0].seq );
        ck_assert_ptr_eq( NULL, query->nt[1].seq );

        ck_assert_ptr_ne( NULL, query->aa[0].seq );
        ck_assert_ptr_eq( NULL, query->aa[1].seq );
        ck_assert_ptr_eq( NULL, query->aa[2].seq );
        ck_assert_ptr_eq( NULL, query->aa[3].seq );
        ck_assert_ptr_eq( NULL, query->aa[4].seq );
        ck_assert_ptr_eq( NULL, query->aa[5].seq );
    }
}

START_TEST (test_read_from_string)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = NUCLEOTIDE;

        char * sequence = "AATTTCGGCTATCTGCTAGCTAGCCTAGCT";

        p_query query = query_read_from_string( sequence );
        ck_sequences_set( query );

        check_query_mapped( 0, map_ncbi_nt16, 30, sequence, query->nt[0] );
        check_query_mapped( 1, map_ncbi_nt16, 30, sequence, query->nt[1] );

        // as defined for strands == 2
        ck_assert_str_eq( query->nt[1].seq, get_reverse_complement( query->nt[0] ) );


        ck_assert_str_eq( sym_ncbi_nt16, query->sym );
        ck_assert_str_eq( map_ncbi_nt16, query->map );

        query_free( query );
    }END_TEST

START_TEST (test_all_allowed_symbols)
    {
        query_strands = BOTH_STRANDS;
        symtype = NUCLEOTIDE;

        char * sequence = "-acmgrsvtwyhkdbn";
        char * expected = "-acmgrsvtwyhkdbn";

        p_query query = query_read_from_string( sequence );
        ck_sequences_set( query );

        check_query_mapped( 0, map_ncbi_nt16, 16, expected, query->nt[0] );
        check_query_mapped( 1, map_ncbi_nt16, 16, expected, query->nt[1] );

        // as defined for strands == 2
        ck_assert_str_eq( query->nt[1].seq, get_reverse_complement( query->nt[0] ) );

        sequence = "-ACMGRSVTWYHKDBN";
        expected = "-ACMGRSVTWYHKDBN";
        query_free( query );

        query = query_read_from_string( sequence );
        ck_sequences_set( query );

        check_query_mapped( 0, map_ncbi_nt16, 16, expected, query->nt[0] );
        check_query_mapped( 1, map_ncbi_nt16, 16, expected, query->nt[1] );

        // as defined for strands == 2
        ck_assert_str_eq( query->nt[1].seq, get_reverse_complement( query->nt[0] ) );

        query_strands = BOTH_STRANDS;
        symtype = AMINOACID;

        sequence = "-ABCDEFGHIKLMNPQRSTVWXYZU*OJ";
        expected = "ABCDEFGHIKLMNPQRSTVWXYZU*OJ";
        query_free( query );

        query = query_read_from_string( sequence );
        ck_sequences_set( query );


        check_query_mapped( 0, map_ncbi_aa, 27, expected, query->aa[0] );

        query_free( query );
    }END_TEST

START_TEST (test_not_allowed_symbols)
    {
        query_strands = FORWARD_STRAND;
        symtype = NUCLEOTIDE;

        char * sequence = "BEX234";

        p_query query = query_read_from_string( sequence );
        check_query_mapped( 0, map_ncbi_nt16, 1, "B", query->nt[0] );

        ck_assert_ptr_eq( NULL, query->nt[1].seq );

        ck_sequences_set( query );

        query_free( query );
    }END_TEST

START_TEST (test_read_from_file_sym0)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = AMINOACID;

        char * sequence =
                "MVQRWLYSTNAKDIAVLYFMLAIFSGMAGTAMSLIIRLELAAPGSQYLHGNSQLFNVLVVG"\
                "HAVLMIFFLVMPALIGGFGNYLLPLMIGATDTAFPRINNIAFWVLPMGLVCLVTSTLVESG"\
                "AGTGWTVYPPLSSIQAHSGPSVDLAIFALHLTSISSLLGAINFIVTTLNMRTNGMTMHKLP"\
                "LFVWSIFITAFLLLLSLPVLSAGITMLLLDRNFNTSFFEVSGGGDPILYEHLFWFFGHPEV"\
                "YILIIPGFGIISHVVSTYSKKPVFGEISMVYAMASIGLLGFLVWSHHMYIVGLDADTRAYF"\
                "TSATMIIAIPTGIKIFSWLATIHGGSIRLATPMLYAIAFLFLFTMGGLTGVALANASLDVA"\
                "FHDTYYVVGHFHYVLSMGAIFSLFAGYYYWSPQILGLNYNEKLAQIQFWLIFIGANVIFFP"\
                "MHFLGINGMPRRIPDYPDAFAGWNYVASIGSFIATLSLFLFIYILYDQLVNGLNNKVNNKS"\
                "VIYNKAPDFVESNTIFNLNTVKSSSIEFLLTSPPAVHSFNTPAVQS";

        p_query query = query_read_from_file( "./tests/testdata/NP_009305.1.fas" );
        ck_assert_ptr_eq( NULL, query->nt[0].seq );
        ck_assert_int_eq( 0, query->nt[0].len );
        ck_assert_ptr_eq( NULL, query->nt[1].seq );
        ck_assert_int_eq( 0, query->nt[1].len );

        check_query_mapped( 0, map_ncbi_aa, 534, sequence, query->aa[0] );

        ck_assert_ptr_ne( NULL, query->aa[0].seq );
        ck_assert_ptr_eq( NULL, query->aa[1].seq );
        ck_assert_ptr_eq( NULL, query->aa[2].seq );
        ck_assert_ptr_eq( NULL, query->aa[3].seq );
        ck_assert_ptr_eq( NULL, query->aa[4].seq );
        ck_assert_ptr_eq( NULL, query->aa[5].seq );

        ck_assert_int_eq( 65, (int )query->headerlen );
        ck_assert_str_eq( "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p", query->header );

        ck_assert_str_eq( sym_ncbi_aa, query->sym );
        ck_assert_str_eq( map_ncbi_aa, query->map );

        query_free( query );
    }END_TEST

START_TEST (test_read_from_file_sym1)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = AMINOACID;

        p_query query = query_read_from_file( "./tests/testdata/NP_009305.1.fas" );

        ck_assert_ptr_eq( NULL, query->nt[0].seq );
        ck_assert_ptr_eq( NULL, query->nt[1].seq );

        ck_assert_ptr_ne( NULL, query->aa[0].seq );
        ck_assert_ptr_eq( NULL, query->aa[1].seq );
        ck_assert_ptr_eq( NULL, query->aa[2].seq );
        ck_assert_ptr_eq( NULL, query->aa[3].seq );
        ck_assert_ptr_eq( NULL, query->aa[4].seq );
        ck_assert_ptr_eq( NULL, query->aa[5].seq );

        ck_assert_int_eq( 65, (int )query->headerlen );
        ck_assert_str_eq( "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p", query->header );

        ck_assert_str_eq( sym_ncbi_aa, query->sym );
        ck_assert_str_eq( map_ncbi_aa, query->map );

        query_free( query );
    }END_TEST

START_TEST (test_read_from_file_sym2)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = TRANS_QUERY;

        us_init_translation( 3, 3 );

        p_query query = query_read_from_file( "./tests/testdata/one_seq.fas" );
        ck_sequences_set( query );

        ck_assert_int_eq( 32, (int )query->headerlen );
        ck_assert_str_eq( "97485665bcded44c4d86c131ca714848", query->header );

        ck_assert_str_eq( sym_ncbi_nt16, query->sym );
        ck_assert_str_eq( map_ncbi_nt16, query->map );

        query_free( query );
    }END_TEST

START_TEST (test_read_from_file_sym3)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = TRANS_DB;

        p_query query = query_read_from_file( "./tests/testdata/NP_009305.1.fas" );
        ck_sequences_set( query );

        ck_assert_int_eq( 65, (int )query->headerlen );
        ck_assert_str_eq( "gi|6226519|ref|NP_009305.1| cytochrome-c oxidase subunit I; Cox1p", query->header );

        ck_assert_str_eq( sym_ncbi_aa, query->sym );
        ck_assert_str_eq( map_ncbi_aa, query->map );

        query_free( query );
    }END_TEST

START_TEST (test_read_from_file_sym4)
    {
        query_strands = COMPLEMENTARY_STRAND;
        symtype = TRANS_BOTH;

        us_init_translation( 3, 3 );

        p_query query = query_read_from_file( "./tests/testdata/one_seq.fas" );
        ck_sequences_set( query );

        ck_assert_int_eq( 32, (int )query->headerlen );
        ck_assert_str_eq( "97485665bcded44c4d86c131ca714848", query->header );

        ck_assert_str_eq( sym_ncbi_nt16, query->sym );
        ck_assert_str_eq( map_ncbi_nt16, query->map );

        query_free( query );
    }END_TEST

START_TEST (test_strands_param)
    {
        // plus
        symtype = TRANS_BOTH;
        query_strands = FORWARD_STRAND;

        us_init_translation( 3, 3 );

        p_query query = query_read_from_file( "./tests/testdata/one_seq.fas" );
        ck_sequences_set( query );

        query_free( query );

        // minus
        query_strands = COMPLEMENTARY_STRAND;

        query = query_read_from_file( "./tests/testdata/one_seq.fas" );
        ck_sequences_set( query );

        query_free( query );

        // both
        query_strands = BOTH_STRANDS;

        query = query_read_from_file( "./tests/testdata/one_seq.fas" );
        ck_sequences_set( query );

        query_free( query );

        // plus
        symtype = NUCLEOTIDE;
        query_strands = FORWARD_STRAND;

        us_init_translation( 3, 3 );

        query = query_read_from_file( "./tests/testdata/one_seq.fas" );
        ck_sequences_set( query );

        query_free( query );

        // minus
        query_strands = COMPLEMENTARY_STRAND;

        query = query_read_from_file( "./tests/testdata/one_seq.fas" );
        ck_sequences_set( query );

        query_free( query );

        // both
        query_strands = BOTH_STRANDS;

        query = query_read_from_file( "./tests/testdata/one_seq.fas" );
        ck_sequences_set( query );

        query_free( query );
    }END_TEST

void addQueryTC( Suite *s ) {
    TCase *tc_core = tcase_create( "query" );
    tcase_add_test( tc_core, test_read_from_string );
    tcase_add_test( tc_core, test_all_allowed_symbols );
    tcase_add_test( tc_core, test_not_allowed_symbols );
    tcase_add_test( tc_core, test_read_from_file_sym0 );
    tcase_add_test( tc_core, test_read_from_file_sym1 );
    tcase_add_test( tc_core, test_read_from_file_sym2 );
    tcase_add_test( tc_core, test_read_from_file_sym3 );
    tcase_add_test( tc_core, test_read_from_file_sym4 );
    tcase_add_test( tc_core, test_strands_param );

    suite_add_tcase( s, tc_core );
}
