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
#include "../src/util/util_sequence.h"
#include "../src/util/util.h"
#include "../src/matrices.h"
#include "../src/algo/searcher.h"
#include "../src/algo/gap_costs.h"
#include "../src/algo/manager.h"

static p_query init_libssa_test( size_t thread_count, char * db_file, char * query_file) {
    init_constant_scores( 5, -4 );
    init_gap_penalties( -4, -2 );
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    set_thread_count( thread_count );

    init_db( db_file );

    return init_sequence_fasta( READ_FROM_FILE, query_file );
}

static void exit_libssa_test( p_alignment_list alist, p_query query ) {
    free_alignment( alist );
    free_sequence( query );
    ssa_exit();
}

START_TEST (test_simple_one_thread)
    {
        p_query query = init_libssa_test( 1, "tests/testdata/AF091148.fas", "tests/testdata/one_seq.fas" );

        p_alignment_list alist = sw_align( query, 5, BIT_WIDTH_64, COMPUTE_ALIGNMENT );
        ck_assert_int_eq( 5, alist->len );

        ck_assert_int_eq( 877, alist->alignments[0]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[0]->score );
        ck_assert_str_eq( "2MD2MIM2I3MD3MD4M2D8M5I3M5IM3I7MI7M3I5MI3M", alist->alignments[0]->alignment );
        ck_assert_int_eq( 847, alist->alignments[1]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[1]->score );
        ck_assert_str_eq( "2MD2MIM2I3MD3MD4M2D8M5I3M5IM3I7MI7M3I4MI4M", alist->alignments[1]->alignment );
        ck_assert_int_eq( 753, alist->alignments[2]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[2]->score );
        ck_assert_str_eq( "2MD2MIM2I3MD3MD3M2D9M5I3M5IM3I7MI7M3I5MI3M", alist->alignments[2]->alignment );
        ck_assert_int_eq( 565, alist->alignments[3]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[3]->score );
        ck_assert_str_eq( "2MD2MIM2I3MD3MD4M2D8M5I3M5IM3I7MI7M3I5MI3M", alist->alignments[3]->alignment );
        ck_assert_int_eq( 398, alist->alignments[4]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[4]->score );
        ck_assert_str_eq( "2MD2MIM2I3MD3MD3M2D9M5I3M5IM3I7MI7M3I5MI3M", alist->alignments[4]->alignment );

        free_alignment( alist );

        alist = nw_align( query, 5, BIT_WIDTH_64, COMPUTE_ALIGNMENT );
        ck_assert_int_eq( 5, alist->len );

        ck_assert_int_eq( 1050, alist->alignments[0]->db_seq.ID );
        ck_assert_int_eq( 33, alist->alignments[0]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M19I4M3IMI3M5I3M5IM3I7MI7M4IMIMI4MI2M7I", alist->alignments[0]->alignment );
        ck_assert_int_eq( 908, alist->alignments[1]->db_seq.ID );
        ck_assert_int_eq( 28, alist->alignments[1]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M5IM2I2M3I2M5IMI9MD2M3I3M9I2M3I2M4I2M2IMI4MI2M7I", alist->alignments[1]->alignment );
        ck_assert_int_eq( 938, alist->alignments[2]->db_seq.ID );
        ck_assert_int_eq( 24, alist->alignments[2]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M5IM2I2M3I2M5IMI9MD2M3I3M9I2M3I2M4IM3IMIMIM2I5M10I", alist->alignments[2]->alignment );
        ck_assert_int_eq( 378, alist->alignments[3]->db_seq.ID );
        ck_assert_int_eq( 21, alist->alignments[3]->score );
        ck_assert_str_eq( "2MI2MI6M3I5M9I2M7IM6IMI3M3I6M5I3M5IM3I7MI7M4IMIMI4MI2M7I", alist->alignments[3]->alignment );
        ck_assert_int_eq( 75, alist->alignments[4]->db_seq.ID );
        ck_assert_int_eq( 12, alist->alignments[4]->score );
        ck_assert_str_eq( "2MI2MI6M3I5M9I2M7IM6IMI3M3I6M5I3M5IM3I7MI7M4IMIMI4MI2M7I", alist->alignments[4]->alignment );

        exit_libssa_test( alist, query );
    }END_TEST

START_TEST (test_sw_multiple_threads)
    {
        p_query query = init_libssa_test( 4, "tests/testdata/AF091148.fas", "tests/testdata/one_seq.fas" );

        p_alignment_list alist = sw_align( query, 5, BIT_WIDTH_64, COMPUTE_ALIGNMENT );
        ck_assert_int_eq( 5, alist->len );

//        ck_assert_int_eq( 877, alist->alignments[0]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[0]->score );
//        ck_assert_str_eq( "2MD2MIM2I3MD3MD4M2D8M5I3M5IM3I7MI7M3I5MI3M", alist->alignments[0]->alignment );
//        ck_assert_int_eq( 847, alist->alignments[1]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[1]->score );
//        ck_assert_str_eq( "2MD2MIM2I3MD3MD4M2D8M5I3M5IM3I7MI7M3I4MI4M", alist->alignments[1]->alignment );
        /*
         * TODO the rest of the test is not testable, since there are more than 3 alignments with a score of 210.
         *
         *      The current algorithms does not give alignments with the same score a particular order, it takes
         *      them, as they come ... here we have to decide, what is best to do in that case!
         */
//        ck_assert_int_eq( 1144, alist->alignments[2]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[2]->score );
//        ck_assert_str_eq( "6M2I3MD3MD3M2D9M5I4M8I7MI5MI4M2I2MI4M", alist->alignments[2]->alignment );
//        ck_assert_int_eq( 965, alist->alignments[3]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[3]->score );
//        ck_assert_str_eq( "4M2I5M3I6MIM2I3MD13M3I7M2I13M", alist->alignments[3]->alignment );
//        ck_assert_int_eq( 313, alist->alignments[4]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[4]->score );
//        ck_assert_str_eq( "7MI6MD3M2D9M5I4M8I7MI5MI4M2I3MI3M", alist->alignments[4]->alignment );

        exit_libssa_test( alist, query );
    }END_TEST

START_TEST (test_nw_multiple_threads)
    {
        p_query query = init_libssa_test( -1, "tests/testdata/AF091148.fas", "tests/testdata/one_seq.fas" );

        p_alignment_list alist = nw_align( query, 5, BIT_WIDTH_64, COMPUTE_ALIGNMENT );
        ck_assert_int_eq( 5, alist->len );

        ck_assert_int_eq( 1050, alist->alignments[0]->db_seq.ID );
        ck_assert_int_eq( 33, alist->alignments[0]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M19I4M3IMI3M5I3M5IM3I7MI7M4IMIMI4MI2M7I", alist->alignments[0]->alignment );
        ck_assert_int_eq( 908, alist->alignments[1]->db_seq.ID );
        ck_assert_int_eq( 28, alist->alignments[1]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M5IM2I2M3I2M5IMI9MD2M3I3M9I2M3I2M4I2M2IMI4MI2M7I", alist->alignments[1]->alignment );
        ck_assert_int_eq( 938, alist->alignments[2]->db_seq.ID );
        ck_assert_int_eq( 24, alist->alignments[2]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M5IM2I2M3I2M5IMI9MD2M3I3M9I2M3I2M4IM3IMIMIM2I5M10I", alist->alignments[2]->alignment );
        ck_assert_int_eq( 378, alist->alignments[3]->db_seq.ID );
        ck_assert_int_eq( 21, alist->alignments[3]->score );
        ck_assert_str_eq( "2MI2MI6M3I5M9I2M7IM6IMI3M3I6M5I3M5IM3I7MI7M4IMIMI4MI2M7I", alist->alignments[3]->alignment );
        ck_assert_int_eq( 75, alist->alignments[4]->db_seq.ID );
        ck_assert_int_eq( 12, alist->alignments[4]->score );
        ck_assert_str_eq( "2MI2MI6M3I5M9I2M7IM6IMI3M3I6M5I3M5IM3I7MI7M4IMIMI4MI2M7I", alist->alignments[4]->alignment );

        exit_libssa_test( alist, query );
    }END_TEST

START_TEST (test_1000_threads)
    {
        set_chunk_size( 1 );

        p_query query = init_libssa_test( 1000, "tests/testdata/AF091148.fas", "tests/testdata/one_seq.fas" );

        p_alignment_list alist = nw_align( query, 5, BIT_WIDTH_64, COMPUTE_ALIGNMENT );
        ck_assert_int_eq( 5, alist->len );

        ck_assert_int_eq( 1050, alist->alignments[0]->db_seq.ID );
        ck_assert_int_eq( 33, alist->alignments[0]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M19I4M3IMI3M5I3M5IM3I7MI7M4IMIMI4MI2M7I", alist->alignments[0]->alignment );
        ck_assert_int_eq( 908, alist->alignments[1]->db_seq.ID );
        ck_assert_int_eq( 28, alist->alignments[1]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M5IM2I2M3I2M5IMI9MD2M3I3M9I2M3I2M4I2M2IMI4MI2M7I", alist->alignments[1]->alignment );
        ck_assert_int_eq( 938, alist->alignments[2]->db_seq.ID );
        ck_assert_int_eq( 24, alist->alignments[2]->score );
        ck_assert_str_eq( "2MI2MI6M3I6MD3M5IM2I2M3I2M5IMI9MD2M3I3M9I2M3I2M4IM3IMIMIM2I5M10I", alist->alignments[2]->alignment );
        ck_assert_int_eq( 378, alist->alignments[3]->db_seq.ID );
        ck_assert_int_eq( 21, alist->alignments[3]->score );
        ck_assert_str_eq( "2MI2MI6M3I5M9I2M7IM6IMI3M3I6M5I3M5IM3I7MI7M4IMIMI4MI2M7I", alist->alignments[3]->alignment );
        ck_assert_int_eq( 75, alist->alignments[4]->db_seq.ID );
        ck_assert_int_eq( 12, alist->alignments[4]->score );
        ck_assert_str_eq( "2MI2MI6M3I5M9I2M7IM6IMI3M3I6M5I3M5IM3I7MI7M4IMIMI4MI2M7I",
                alist->alignments[4]->alignment );

        free_alignment( alist );

        alist = sw_align( query, 5, BIT_WIDTH_64, COMPUTE_ALIGNMENT );
        ck_assert_int_eq( 5, alist->len );

//        ck_assert_int_eq( 1228, alist->alignments[0]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[0]->score );
//        ck_assert_str_eq( "2MD2MIM2I3MD3MD3M2D9M5I3M5IM3I7MI7M3I5MI3M", alist->alignments[0]->alignment );
//        ck_assert_int_eq( 1144, alist->alignments[1]->db_seq.ID );
        ck_assert_int_eq( 91, alist->alignments[1]->score );
//        ck_assert_str_eq( "6M2I3MD3MD3M2D9M5I4M8I7MI5MI4M2I3MI3M", alist->alignments[1]->alignment );
        /*
         * TODO the rest of the test is not testable, since there are more than 3 alignments with a score of 163.
         *
         *      The current algorithms does not give alignments with the same score a particular order, it takes
         *      them, as they come ... here we have to decide, what is best to do in that case!
         */
//        ck_assert_int_eq(1340, alist->alignments[2]->db_seq.ID);
        ck_assert_int_eq(91, alist->alignments[2]->score);
//        ck_assert_str_eq("6M2I3MD3MD3M2D9M5I4M8I7MI5MI4M2I3MI3M", alist->alignments[2]->alignment);
//        ck_assert_int_eq(965, alist->alignments[3]->db_seq.ID);
        ck_assert_int_eq(91, alist->alignments[3]->score);
//        ck_assert_str_eq("7M2I3MD3MD4M2D8M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[3]->alignment);
//        ck_assert_int_eq(1164, alist->alignments[4]->db_seq.ID);
        ck_assert_int_eq(91, alist->alignments[4]->score);
//        ck_assert_str_eq("7M2I3MD3MD4M2D8M5I4M8I7MI5MI4M2I3MI2M", alist->alignments[4]->alignment);

        exit_libssa_test( alist, query );
    }END_TEST

START_TEST (test_init_functions)
    {
        set_output_mode( OUTPUT_SILENT );
        set_output_mode( OUTPUT_INFO );
        set_output_mode( OUTPUT_WARNING );
        set_output_mode( OUTPUT_ERROR );

        set_simd_compute_mode( COMPUTE_ON_AVX2 );
        set_simd_compute_mode( COMPUTE_ON_SSE41 );
        set_simd_compute_mode( COMPUTE_ON_SSE2 );

        set_chunk_size( 0 );
        ck_assert_int_eq( DEFAULT_CHUNK_SIZE, max_chunk_size );

        init_score_matrix( MATRIX_BUILDIN, BLOSUM45 );
        init_score_matrix( MATRIX_BUILDIN, BLOSUM50 );
        init_score_matrix( MATRIX_BUILDIN, BLOSUM62 );
        init_score_matrix( MATRIX_BUILDIN, BLOSUM80 );
        init_score_matrix( MATRIX_BUILDIN, BLOSUM90 );
        init_score_matrix( MATRIX_BUILDIN, PAM30 );
        init_score_matrix( MATRIX_BUILDIN, PAM70 );
        init_score_matrix( MATRIX_BUILDIN, PAM250 );

        init_score_matrix( READ_FROM_FILE, "tests/testdata/blosum90.txt" );

        init_score_matrix( READ_FROM_STRING, mat_blosum80 );

        init_gap_penalties( -8, -5 );
        ck_assert_int_eq( -8, gapO );
        ck_assert_int_eq( -5, gapE );

        init_constant_scores( 3, 6 );

        init_symbol_translation( NUCLEOTIDE, BOTH_STRANDS, 3, 3 );
        ck_assert_int_eq( NUCLEOTIDE, symtype );
        ck_assert_int_eq( BOTH_STRANDS, query_strands );

        init_db( "tests/testdata/AF091148.fas" );
        ck_assert_int_eq( 1403, ssa_db_get_sequence_count() );

        p_query query = init_sequence_fasta( READ_FROM_FILE, "tests/testdata/one_seq.fas" );
        ck_assert_str_eq( "97485665bcded44c4d86c131ca714848", query->header );
        free_sequence( query );

        query = init_sequence_fasta( READ_FROM_STRING, "ATGCCCAAAATTGACTTGAATAGGGGCGT" );
        ck_assert_ptr_ne( NULL, query->nt[0].seq );

        exit_libssa_test( NULL, query );
    }END_TEST

void addLibssaTC( Suite *s ) {
    TCase *tc_core = tcase_create( "libssa" );
    tcase_add_test( tc_core, test_simple_one_thread );
    tcase_add_test( tc_core, test_sw_multiple_threads );
    tcase_add_test( tc_core, test_nw_multiple_threads );
    tcase_add_test( tc_core, test_1000_threads );
    tcase_add_test( tc_core, test_init_functions );

    // TODO add 8/16 bit SSE/AVX test

    suite_add_tcase( s, tc_core );
}
