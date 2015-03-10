/*
 * needleman_wunsch_64.c
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../../matrices.h"
#include "../../util/util.h"
#include "../gap_costs.h"


int64_t full_nw( sequence_t * dseq, sequence_t * qseq, int64_t * hearray ) {
    int64_t h; // current value
    int64_t n; // diagonally previous value
    int64_t e; // value in left cell
    int64_t f; // value in upper cell
    int64_t *hep;

    for( size_t i = 0; i < qseq->len; i++ ) {
        hearray[2 * i] = -gapO + (i + 1) * -gapE;           // H (N) scores in previous column
        hearray[2 * i + 1] = 2 * -gapO + (i + 2) * -gapE;   // E     gap values in previous column
    }

#ifdef DBG_COLLECT_MATRIX
        dbg_init_matrix_data_collection( BIT_WIDTH_64, dseq->len, qseq->len );
#endif

    for( size_t j = 0; j < dseq->len; j++ ) {
        hep = hearray;

        f = 2 * -gapO + (j + 2) * -gapE;        // value in first upper cell
        h = (j == 0) ? 0 : (-gapO + j * -gapE); // value in first cell of line

        for( size_t i = 0; i < qseq->len; i++ ) {
            n = *hep;
            e = *(hep + 1);
            h += SCORE_MATRIX_64( dseq->seq[j], qseq->seq[i] );

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

#ifdef DBG_COLLECT_MATRIX
            dbg_add_matrix_data_64( i, j, *hep, e, f );
#endif

            // next round
            *(hep + 1) = e;
            h = n;
            hep += 2;
        }
    }

#ifdef DBG_COLLECT_MATRIX
        sequence_t * db_sequences = xmalloc( sizeof( sequence_t ) );
        db_sequences[0] = *dseq;

        dbg_print_matrices_to_file( BIT_WIDTH_64, "NW", qseq->seq, db_sequences, 1 );

        free( db_sequences );
#endif

    return hearray[2 * qseq->len - 2];;
}
