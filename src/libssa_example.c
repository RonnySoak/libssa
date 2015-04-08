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
    p_alignment_list alist = align_func( query, hit_count, bit_width, COMPUTE_SCORE );

    printf( "Nr of alignments: %ld\n", alist->len );

//    for( size_t i = 0; i < alist->len; i++ ) {
//        p_alignment a = alist->alignments[i];
//
//        printf( "DB-ID %ld, score: %ld, cigar: %s\n", a->db_seq.ID, a->score, a->alignment );
//    }

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

    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );

    set_threads( 8 );

    set_output_mode( OUTPUT_INFO );

    init_score_matrix( MATRIX_BUILDIN, "BLOSUM50" );
    init_gap_penalties( -3, -1 );
    init_db_fasta( "tests/testdata/uniprot_sprot.fasta" );
//    init_db_fasta( "tests/testdata/AF091148.fas" );

    p_query query_1 = init_sequence_fasta( READ_FROM_FILE, "tests/testdata/O74807.fasta" );
    p_query query_2 = init_sequence_fasta( READ_FROM_FILE, "tests/testdata/P19930.fasta" );
    p_query query_3 = init_sequence_fasta( READ_FROM_FILE, "tests/testdata/Q3ZAI3.fasta" );
    p_query query_4 = init_sequence_fasta( READ_FROM_FILE, "tests/testdata/P18080.fasta" );
    size_t hit_count = 10;

    p_alignment_list alist = do_alignment( "overflow test: O74807", &sw_align, query_1, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    alist = do_alignment( "overflow test: P19930", &sw_align, query_2, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    alist = do_alignment( "overflow test: Q3ZAI3", &sw_align, query_3, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    alist = do_alignment( "overflow test: P18080", &sw_align, query_4, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    alist = do_alignment( "overflow test: O74807", &nw_align, query_1, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    alist = do_alignment( "overflow test: P19930", &nw_align, query_2, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    alist = do_alignment( "overflow test: Q3ZAI3", &nw_align, query_3, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    alist = do_alignment( "overflow test: P18080", &nw_align, query_4, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );

    p_query query_5 = init_sequence_fasta( READ_FROM_FILE, "benchmark/data/P56980" );
    alist = do_alignment( "overflow test: P56980", &nw_align, query_5, hit_count, BIT_WIDTH_8 );
    free_alignment( alist );
    free_sequence( query_5 );

    free_sequence( query_1 );
    free_sequence( query_2 );
    free_sequence( query_3 );
    free_sequence( query_4 );

    ssa_exit();

    return 0;
}
