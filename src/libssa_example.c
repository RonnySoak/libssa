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

#include "libssa.h"

#include <stdio.h>

static p_alignment_list do_alignment( char * desc, p_alignment_list (*align_func)( p_query, size_t, int, int ), p_query query,
        size_t hit_count, int bit_width ) {

    printf( "%s: \n", desc );
    p_alignment_list alist = align_func( query, hit_count, bit_width, COMPUTE_ALIGNMENT );

    printf( "Nr of alignments: %ld\n", alist->len );

    for( size_t i = 0; i < alist->len; i++ ) {
        p_alignment a = alist->alignments[i];

        printf( "DB-ID %ld, score: %ld, cigar: %s\n", a->db_seq.ID, a->score, a->alignment );
    }

//    for( size_t i = 0; i < alist->len; i++ ) {
//        alignment_p a = alist->alignments[i];
//
//        printf( "%ld, %ld, ", a->score, a->db_seq.ID );
//    }
//    printf( "\n" );

    return alist;
}

//static void cmp_alignment_lists( p_alignment_list a, p_alignment_list b, size_t hit_count, char * desc ) {
//    int difference = 0;
//
//    for( size_t i = 0; i < hit_count; ++i ) {
//        if( a->alignments[i]->score != b->alignments[i]->score
//                || a->alignments[i]->db_seq.ID != b->alignments[i]->db_seq.ID ) {
//            difference = 1;
//        }
//    }
//
//    if( difference )
//        printf( "%s differ\n", desc );
//}

int main( int argc, char**argv ) {
    set_simd_compute_mode( COMPUTE_ON_SSE41 );

    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    set_threads( 1 );

    set_output_mode( OUTPUT_SILENT );

    init_score_matrix( MATRIX_BUILDIN, "blosum62" );
    init_gap_penalties( 3, 1 );
//    init_db_fasta( "tests/testdata/uniprot_sprot.fasta" );
    init_db_fasta( "tests/testdata/AF091148.fas" );

    p_query query = init_sequence_fasta( READ_FROM_FILE, "tests/testdata/O74807.fasta" );
    size_t hit_count = 10;

    p_alignment_list alist = do_alignment( "overflow test", &sw_align, query, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    free_sequence( query );

//    init_constant_scoring( 1, -1 );
//    init_gap_penalties( 0, 1 );
////    init_db_fasta( "tmp/3a.fas" );
//    p_query query = init_sequence_fasta( READ_FROM_STRING, "TACGGGTAT" );
////
//    size_t hit_count = 1;
////
//    free_alignment( alist );
//
//    alist = do_alignment( "3.b", &sw_align, query, hit_count, BIT_WIDTH_64 );
//    free_alignment( alist );
//
//    init_gap_penalties( 20, 1 );
//
//    alist = do_alignment( "3.c", &nw_align, query, hit_count, BIT_WIDTH_64 );
//    free_alignment( alist );
//
//    free_sequence( query );
//
//
//    init_db_fasta( "tmp/3d.fas" );
//    query = init_sequence_fasta( READ_FROM_STRING, "AAGT" );
//    init_gap_penalties( 1, 1 );
//
//    alist = do_alignment( "3.d", &nw_align, query, hit_count, BIT_WIDTH_64 );
//    free_alignment( alist );
//
//    free_sequence( query );
//
//    init_db_fasta( "tmp/3e.fas" );
//    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );
//    query = init_sequence_fasta( READ_FROM_STRING, "MRWAC" );
//
//    init_constant_scoring( 2, 0 );
//    init_gap_penalties( 0, 1 );
//
//    alist = do_alignment( "3.e", &nw_align, query, hit_count, BIT_WIDTH_64 );
//    free_alignment( alist );
//
//    free_sequence( query );
//
//
//    init_db_fasta( "tmp/3f.fas" );
//    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );
//    query = init_sequence_fasta( READ_FROM_STRING, "EGGHVLVAV" );
//
//    init_score_matrix( MATRIX_BUILDIN, BLOSUM62 );
//    init_gap_penalties( 0, 5 );
//
//    alist = do_alignment( "3.f", &sw_align, query, hit_count, BIT_WIDTH_64 );
//    free_alignment( alist );
//
//    init_gap_penalties( 4, 1 );
//
//    alist = do_alignment( "3.g", &sw_align, query, hit_count, BIT_WIDTH_64 );
//    free_alignment( alist );
//
//    free_sequence( query );


//    init_db_fasta( "tmp/ALKB2_HUMAN.txt" );
//    init_symbol_translation( AMINOACID, FORWARD_STRAND, 1, 1 );
//    query = init_sequence_fasta( READ_FROM_FILE, "tmp/np_001264426.txt" );
//
//    init_score_matrix( MATRIX_BUILDIN, BLOSUM62 );
//    init_gap_penalties( 11, 1 );
//
//    alist = do_alignment( "home exam", &sw_align, query, hit_count, BIT_WIDTH_64 );
//    free_alignment( alist );
//
//    alist = do_alignment( "home exam", &sw_align, query, hit_count, BIT_WIDTH_16 );
//    free_alignment( alist );
//
//    alist = do_alignment( "home exam", &sw_align, query, hit_count, BIT_WIDTH_8 );
//    free_alignment( alist );

    ssa_exit();

    return 0;
}
