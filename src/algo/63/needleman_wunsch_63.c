/*
 * needleman_wunsch_63.c
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../search.h"

#include "../../matrices.h"
#include "../../util/util_sequence.h" // TODO remove
//#include <stdlib.h>  // TODO remove
//#include <stdio.h>  // TODO remove

//int BIT_GAP_DATABASE = 0;
//int BIT_GAP_QUERY = 1;
//int BIT_MIS_MATCH = 2;
//
//int ** H;
//int ** D;
//
//static void init_H_D( int d_len, int q_len ) { // TODO remove
//    H = calloc( d_len, sizeof(int*) );
//
//    for( int i = 0; i < d_len; i++ ) {
//        /* size_y is the height */
//        H[i] = calloc( q_len, sizeof(int) );
//    }
//
//    D = calloc( d_len, sizeof(int*) );
//
//    for( int i = 0; i < d_len; i++ ) {
//        /* size_y is the height */
//        D[i] = calloc( q_len, sizeof(int) );
//    }
//}
//
//static void print_matrix( int ** m, sequence * d, sequence * q ) {
//    int id, iq;
//
//    // first line seq1
//    printf( "        " );
//    for( id = 0; id < (q->len - 1); id++ ) {
//        printf( "    %c", sym_ncbi_nt16u[(int )q->seq[id]] );
//    }
//    printf( "\n" );
//
//    // second is first line of matrix
//    printf( "   " );
//    for( iq = 0; iq < q->len; iq++ ) {
//        printf( "  %3d", m[0][iq] );
//    }
//    printf( "\n" );
//
//    // rest of matrix and seq2
//    for( id = 1; id < d->len; id++ ) {
//        printf( "  %c", sym_ncbi_nt16u[(int )d->seq[id - 1]] );
//        for( iq = 0; iq < q->len; iq++ ) {
//            printf( "  %3d", m[id][iq] );
//        }
//        printf( "\n" );
//    }
//}


int64_t full_nw( sequence * dseq, sequence * qseq, int64_t * hearray ) {
//    init_H_D( dseq->len, qseq->len );

    int64_t h; // current value
    int64_t n; // diagonally previous value
    int64_t e; // value in left cell
    int64_t f; // value in upper cell
    int64_t *hep;

    for( uint64_t i = 0; i < qseq->len; i++ ) {
//        hearray[2 * i] = -gapO + (i + 1) * -gapE;
//        hearray[2 * i + 1] = 2 * -gapO + (i + 2) * -gapE; TODO like in SWARM

        hearray[2 * i] = -gapO + (i + 1) * -gapE; // H (N) scores in previous column
        hearray[2 * i + 1] = hearray[2 * i];      // E     gap values in previous column
    }

    for( uint64_t j = 0; j < dseq->len; j++ ) {
        hep = hearray;

//        f = 2 * -gapO + (j+2) * -gapE;
//        h = (j == 0) ? 0 : (-gapO + j * -gapE); TODO like in SWARM

        f = -gapO + (j + 1) * -gapE;            // value in first upper cell
        h = (j == 0) ? 0 : (-gapO + j * -gapE); // value in first cell of line

//        H[j][1] = f;
//        printf("h %d, f %d\n", h, f);

        for( uint64_t i = 0; i < qseq->len; i++ ) {
//            H[0][i] = h;

            n = *hep;
            e = *(hep + 1);
            h += SCORE_MATRIX_63( dseq->seq[j], qseq->seq[i] );

            // test for gap opening
            if( f > h )
                h = f;
            if( e > h )
                h = e;

            *hep = h;

            // test for gap extensions
            e += -gapE;
            f += -gapE;
            h += -gapO + -gapE;

            if( f < h )
                f = h;
            if( e < h )
                e = h;

            // next round
            *(hep + 1) = e;
            h = n;
            hep += 2;
        }
    }

//    printf( "Matrix:\n" );
//    print_matrix( H, dseq, qseq );
//
//    printf( "Directions:\n" );
//    print_matrix( D, dseq, qseq );

//    free( H );
//    free( D );

    return hearray[2 * qseq->len - 2];;
}
