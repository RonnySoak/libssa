/*
 * libssa_example.c
 *
 *  Created on: Sep 4, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "libssa.h"
#include "cpu_config.h"

#include <stdio.h>

static p_alignment_list do_alignment( char * desc, p_alignment_list (* align_func)( p_query, int, int ),
        p_query query, int hit_count, int bit_width ) {

    printf( "%s: \n", desc );
    p_alignment_list alist = align_func( query, hit_count, bit_width );

    printf( "Nr of alignments: %ld\n", alist->len );

    for( int i = 0; i < alist->len; i++ ) {
        alignment_p a = alist->alignments[i];

        printf( "DB-ID %ld, score: %ld, cigar: %s\n", a->db_seq.ID, a->score, a->alignment );
    }
    return alist;
}

static void cmp_alignment_lists( p_alignment_list a, p_alignment_list b, int hit_count, char * desc ) {
    int difference = 0;

    for( int i = 0; i < hit_count; ++i ) {
        if( a->alignments[i]->score != b->alignments[i]->score
                || a->alignments[i]->db_seq.ID != b->alignments[i]->db_seq.ID ) {
            difference = 1;
        }
    }

    if( difference )
        printf( "%s differ\n", desc );
}

int main( int argc, char**argv ) {
    set_max_compute_capability( COMPUTE_ON_SSE41 ); // TODO make configurable via API

    init_scoring( 2, -2 );
    init_gap_penalties( 1, 1 );
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    set_threads( 1 );

//    init_db_fasta("tests/testdata/test.fas");
//    init_db_fasta("tests/testdata/test_16_seq.fas");
    init_db_fasta( "tests/testdata/AF091148.fas" );
//    init_db_fasta( "tests/testdata/AF091148_selection.fas" );
//    init_db_fasta("tests/testdata/Rfam_11_0.fasta");

    p_query query = init_sequence_fasta( "tests/testdata/one_seq.fas" );

    int hit_count = 10;

    p_alignment_list alist_sw_64 = do_alignment( "Do local alignment using 64 bit Smith-Waterman", &sw_align, query, hit_count, BIT_WIDTH_64 );
    p_alignment_list alist_nw_64 = do_alignment( "Do global alignment using 64 bit Needleman-Wunsch", &nw_align, query, hit_count, BIT_WIDTH_64 );

    p_alignment_list alist_sw_16 = do_alignment( "Do local alignment using 16 bit Smith-Waterman", &sw_align, query, hit_count, BIT_WIDTH_16 );
    p_alignment_list alist_nw_16 = do_alignment( "Do global alignment using 16 bit Needleman-Wunsch", &nw_align, query, hit_count, BIT_WIDTH_16 );

    p_alignment_list alist_sw_8 = do_alignment( "Do local alignment using 8 bit Smith-Waterman", &sw_align, query, hit_count, BIT_WIDTH_8 );
    p_alignment_list alist_nw_8 = do_alignment( "Do global alignment using 8 bit Needleman-Wunsch", &nw_align, query, hit_count, BIT_WIDTH_8 );

    cmp_alignment_lists( alist_nw_64, alist_nw_16, hit_count, "NW_64_16" );
    cmp_alignment_lists( alist_nw_64, alist_nw_8, hit_count, "NW_64_8" );
    cmp_alignment_lists( alist_nw_16, alist_nw_8, hit_count, "NW_16_8" );
    cmp_alignment_lists( alist_sw_64, alist_sw_16, hit_count, "SW_64_16" );
    cmp_alignment_lists( alist_sw_64, alist_sw_8, hit_count, "SW_64_8" );
    cmp_alignment_lists( alist_sw_16, alist_sw_8, hit_count, "SW_16_8" );

    free_alignment( alist_nw_64 );
    free_alignment( alist_sw_64 );
    free_alignment( alist_nw_16 );
    free_alignment( alist_sw_16 );
    free_alignment( alist_nw_8 );
    free_alignment( alist_sw_8 );


    free_db();
    free_sequence( query );

    ssa_exit();

    return 0;
}
