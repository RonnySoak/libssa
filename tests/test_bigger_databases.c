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
#include "../src/query.h"
#include "../src/matrices.h"
#include "../src/algo/searcher.h"
#include "../src/cpu_config.h"

static p_query setup_bigger_db_test( char * db_file, char * query_file, int8_t gapO, int8_t gapE,
        size_t thread_count, int symtype ) {
    set_max_compute_capability( COMPUTE_ON_AVX2 );

    init_gap_penalties( gapO, gapE );
    init_symbol_translation( symtype, FORWARD_STRAND, 3, 3 );

    set_thread_count( thread_count );

    init_db_fasta( concat( "tests/testdata/", db_file ) );

    return init_sequence_fasta( READ_FROM_FILE, concat( "tests/testdata/", query_file ) );
}

static void ck_alignment( p_alignment_list alist, size_t hit_count, long * expected_alignment ) {
    ck_assert_int_eq( hit_count, alist->len );

    /*
     * Result contains tuple of the score and the database ID
     */
    for( int i = 0; i < (hit_count * 2); i += 2 ) {
        ck_assert_int_eq( expected_alignment[i], alist->alignments[i / 2]->score );
        ck_assert_int_eq( expected_alignment[i + 1], alist->alignments[i / 2]->db_seq.ID );
    }
}

static void do_alignment( p_alignment_list (*align_func)( p_query, size_t, int, int ), p_query query, size_t hit_count,
        int bit_width, long * exp_al_data ) {
    p_alignment_list alist = align_func( query, hit_count, bit_width, COMPUTE_SCORE );

    // 64 bit
    ck_alignment( alist, hit_count, exp_al_data );

    free_alignment( alist );
}

static void exit_bigger_db_test( p_query query ) {
    free_sequence( query );

    ssa_exit();
}

START_TEST (test_AF091148)
    {
        size_t hit_count = 10;
        init_constant_scores( 2, -2 );

        p_query query = setup_bigger_db_test( "AF091148.fas", "one_seq.fas", -4, -2, 1, NUCLEOTIDE );

        long exp_al_nw_64[] = { -88, 908, -92, 1050, -94, 938, -98, 378, -102, 75, -104, 23, -106, 1229, -110, 1148, -110, 612, -112, 1016, };
        long exp_al_nw_16[] = { -88, 908, -92, 1050, -94, 938, -98, 378, -102, 75, -104, 23, -106, 1229, -110, 1148, -110, 612, -112, 1016, };
        long exp_al_nw__8[] = { -88, 908, -92, 1050, -94, 938, -98, 378, -102, 75, -104, 23, -106, 1229, -110, 1148, -110, 612, -112, 1016, };
        long exp_al_sw_64[] = { 20, 1177, 18, 384, 18, 277, 18, 215, 18, 214, 18, 210, 18, 151, 18, 126, 18, 114, 18, 101, };
        long exp_al_sw_16[] = { 20, 1177, 18, 384, 18, 277, 18, 215, 18, 214, 18, 210, 18, 151, 18, 126, 18, 114, 18, 101, };
        long exp_al_sw__8[] = { 20, 1177, 18, 384, 18, 277, 18, 215, 18, 214, 18, 210, 18, 151, 18, 126, 18, 114, 18, 101, };

        do_alignment( &nw_align, query, hit_count, BIT_WIDTH_64, exp_al_nw_64 );
        do_alignment( &sw_align, query, hit_count, BIT_WIDTH_64, exp_al_sw_64 );
        do_alignment( &nw_align, query, hit_count, BIT_WIDTH_16, exp_al_nw_16 );
        do_alignment( &sw_align, query, hit_count, BIT_WIDTH_16, exp_al_sw_16 );
        do_alignment( &nw_align, query, hit_count, BIT_WIDTH_8, exp_al_nw__8 );
        do_alignment( &sw_align, query, hit_count, BIT_WIDTH_8, exp_al_sw__8 );

        exit_bigger_db_test( query );
    }END_TEST

START_TEST (test_Rfam_11_0)
    {
        /*
         * Test requires the Rfam database version 11
         */
        size_t hit_count = 10;
        init_constant_scores( 5, -4 );

        p_query query = setup_bigger_db_test( "Rfam_11_0.fasta", "one_seq.fas", -4, -2, 1, NUCLEOTIDE );

        long exp_al_nw_16[] = { 102, 335336, 95, 117010, 90, 190314, 90, 106501, 89, 106713, 89, 78465, 88, 101449, 87, 364612, 87, 103759, 85, 101161, };
        long exp_al_nw__8[] = { 102, 335336, 95, 117010, 90, 190314, 90, 106501, 89, 106713, 89, 78465, 88, 101449, 87, 364612, 87, 103759, 85, 101161, };
        long exp_al_sw_16[] = { 137, 338662, 137, 338657, 128, 208362, 126, 229662, 125, 367196, 124, 367253, 124, 235578, 124, 201861, 124, 129490, 123, 117444, };
        long exp_al_sw__8[] = { 137, 338662, 137, 338657, 128, 208362, 126, 229662, 125, 367196, 124, 367253, 124, 235578, 124, 201861, 124, 129490, 123, 117444, };

        /*
         * We cannot run the test with this DB and 64 bit, since it takes to long, and we run into a timeout
         */
        do_alignment( &nw_align, query, hit_count, BIT_WIDTH_16, exp_al_nw_16 );
        do_alignment( &sw_align, query, hit_count, BIT_WIDTH_16, exp_al_sw_16 );
        do_alignment( &nw_align, query, hit_count, BIT_WIDTH_8, exp_al_nw__8 );
        do_alignment( &sw_align, query, hit_count, BIT_WIDTH_8, exp_al_sw__8 );

        exit_bigger_db_test( query );
    }END_TEST

START_TEST (test_uniprot_sprot_sw)
    {
        /*
         * Test requires the UniprotKB/Swiss-Prot database revision 2015_03
         */
        size_t hit_count = 10;
        init_score_matrix( MATRIX_BUILDIN, BLOSUM62 );

        p_query query = setup_bigger_db_test( "uniprot_sprot.fasta", "O74807.fasta", -4, -2, 8, AMINOACID );

        long exp_al_sw_16[] = { 567, 232080, 144, 57785, 135, 57784, 130, 57781, 129, 57782, 128, 57783, 112, 232079, 104, 536355, 98, 539010, 98, 57789, };
        long exp_al_sw__8[] = { 567, 232080, 144, 57785, 135, 57784, 130, 57781, 129, 57782, 128, 57783, 112, 232079, 104, 536355, 98, 539010, 98, 57789, };

        /*
         * We cannot run the test with this DB and 64 bit, since it takes to long, and we run into a timeout
         */
        do_alignment( &sw_align, query, hit_count, BIT_WIDTH_16, exp_al_sw_16 );
        do_alignment( &sw_align, query, hit_count, BIT_WIDTH_8, exp_al_sw__8 );

        exit_bigger_db_test( query );
    }END_TEST

START_TEST (test_uniprot_sprot_nw)
    {
        /*
         * Test requires the UniprotKB/Swiss-Prot database revision 2015_03
         */
        size_t hit_count = 10;
        init_score_matrix( MATRIX_BUILDIN, BLOSUM62 );

        p_query query = setup_bigger_db_test( "uniprot_sprot.fasta", "O74807.fasta", -4, -2, 8, AMINOACID );

        long exp_al_nw_16[] = { 567, 232080, 101, 57785, 93, 57781, 82, 57783, 77, 57784, 77, 57782, 69, 232079, 53, 117510, 51, 232081, 49, 186660, };
        long exp_al_nw__8[] = { 567, 232080, 101, 57785, 93, 57781, 82, 57783, 77, 57784, 77, 57782, 69, 232079, 53, 117510, 51, 232081, 49, 186660, };

        /*
         * We cannot run the test with this DB and 64 bit, since it takes to long, and we run into a timeout
         */
        do_alignment( &nw_align, query, hit_count, BIT_WIDTH_16, exp_al_nw_16 );
        do_alignment( &nw_align, query, hit_count, BIT_WIDTH_8, exp_al_nw__8 );

        exit_bigger_db_test( query );
    }END_TEST

void addBiggerDatabasesTC( Suite *s ) {
    TCase *tc_core = tcase_create( "libssa_bigger_databases" );
    tcase_add_test( tc_core, test_AF091148 );
    tcase_add_test( tc_core, test_Rfam_11_0 );
    tcase_add_test( tc_core, test_uniprot_sprot_sw );
    tcase_add_test( tc_core, test_uniprot_sprot_nw );

    tcase_set_timeout( tc_core, 30 );

    suite_add_tcase( s, tc_core );
}
