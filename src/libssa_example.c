/*
 * libssa_example.c
 *
 *  Created on: Sep 4, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "libssa.h"

#include <stdio.h>

static void do_alignment( char * desc, p_alignment_list (* align_func)( p_query, int, int ),
        p_query query, int hit_count, int bit_width ) {

    printf( "%s: \n", desc );
    p_alignment_list alist = align_func( query, hit_count, bit_width );

    printf( "Nr of alignments: %ld\n", alist->len );

    for( int i = 0; i < alist->len; i++ ) {
        alignment_p a = alist->alignments[i];

        printf( "DB-ID %ld, score: %ld, cigar: %s\n", a->db_seq.ID, a->score, a->alignment );
    }
    free_alignment( alist );
}

int main( int argc, char**argv ) {
    /*
     * TODO
     *
     * - find a saturated add for adding signed values
     * - or add a bias to have the matrix positive and deduct that bias after adding the matrix value
     * - check if score == MAX_INT, than we went out of bounds of our saturated operation
     * - with the bias we have to check for MAX_INT - bias
     * - or use signed operations and treat -MAX_INT as 0 and add the bias later on, to get the real score
     *
     */

    init_scoring( 2, -2 );
    init_gap_penalties( 1, 1 );
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    set_threads( 4 );

//    init_db_fasta("tests/testdata/test.fas");
    init_db_fasta( "tests/testdata/AF091148.fas" );
//    init_db_fasta("tests/testdata/Rfam_11_0.fasta");

    p_query query = init_sequence_fasta( "tests/testdata/one_seq.fas" );

    int hit_count = 5;

    do_alignment( "Do local alignment using 64 bit Smith-Waterman", &sw_align, query, hit_count, BIT_WIDTH_64 );
    do_alignment( "Do global alignment using 64 bit Needleman-Wunsch", &nw_align, query, hit_count, BIT_WIDTH_64 );
    do_alignment( "Do local alignment using 16 bit Smith-Waterman", &sw_align, query, hit_count, BIT_WIDTH_16 );
    do_alignment( "Do global alignment using 16 bit Needleman-Wunsch", &nw_align, query, hit_count, BIT_WIDTH_16 );

    free_db();
    free_sequence( query );

    ssa_exit();

    return 0;
}
