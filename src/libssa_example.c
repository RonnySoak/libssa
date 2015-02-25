/*
 * libssa_example.c
 *
 *  Created on: Sep 4, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "libssa.h"
#include "cpu_config.h"

#include <stdio.h>

static p_alignment_list do_alignment( char * desc, p_alignment_list (*align_func)( p_query, size_t, int, int ), p_query query,
        size_t hit_count, int bit_width ) {

    printf( "%s: \n", desc );
    p_alignment_list alist = align_func( query, hit_count, bit_width, COMPUTE_ALIGNMENT );

    printf( "Nr of alignments: %ld\n", alist->len );

    for( size_t i = 0; i < alist->len; i++ ) {
        alignment_p a = alist->alignments[i];

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
    set_max_compute_capability( COMPUTE_ON_SSE41 ); // TODO make configurable via API

    init_constant_scoring( 1, -1 );
    init_gap_penalties( 0, 1 );
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    set_threads( 1 );

    init_db_fasta( "tmp/3a.fas" );
    p_query query = init_sequence_fasta( READ_FROM_STRING, "TACGGGTAT" );

    int hit_count = 1;

    p_alignment_list alist = do_alignment( "3.a", &nw_align, query, hit_count, BIT_WIDTH_64 );
    free_alignment( alist );

    alist = do_alignment( "3.b", &sw_align, query, hit_count, BIT_WIDTH_64 );
    free_alignment( alist );

    init_gap_penalties( 20, 1 );

    alist = do_alignment( "3.c", &nw_align, query, hit_count, BIT_WIDTH_64 );
    free_alignment( alist );

    free_sequence( query );


    init_db_fasta( "tmp/3d.fas" );
    query = init_sequence_fasta( READ_FROM_STRING, "AAGT" );
    init_gap_penalties( 1, 1 );

    alist = do_alignment( "3.d", &nw_align, query, hit_count, BIT_WIDTH_64 );
    free_alignment( alist );

    free_sequence( query );

    init_db_fasta( "tmp/3e.fas" );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );
    query = init_sequence_fasta( READ_FROM_STRING, "MRWAC" );

    init_constant_scoring( 2, 0 );
    init_gap_penalties( 0, 1 );

    alist = do_alignment( "3.e", &nw_align, query, hit_count, BIT_WIDTH_64 );
    free_alignment( alist );

    free_sequence( query );


    init_db_fasta( "tmp/3f.fas" );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );
    query = init_sequence_fasta( READ_FROM_STRING, "EGGHVLVAV" );

    init_score_matrix( MATRIX_BUILDIN, BLOSUM62 );
    init_gap_penalties( 0, 5 );

    alist = do_alignment( "3.f", &sw_align, query, hit_count, BIT_WIDTH_64 );
    free_alignment( alist );

    init_gap_penalties( 4, 1 );

    alist = do_alignment( "3.g", &sw_align, query, hit_count, BIT_WIDTH_64 );
    free_alignment( alist );

    free_sequence( query );

    ssa_exit();

    return 0;
}
