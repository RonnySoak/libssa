/*
 SWIPE
 Smith-Waterman database searches with Inter-sequence Parallel Execution

 Copyright (C) 2008-2012 Torbjorn Rognes, University of Oslo,
 Oslo University Hospital and Sencel Bioinformatics AS

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

 Contact: Torbjorn Rognes <torognes@ifi.uio.no>,
 Department of Informatics, University of Oslo,
 PO Box 1080 Blindern, NO-0316 Oslo, Norway
 */

#include "../search.h"

#include <string.h>

#include "../../util/util.h"
#include "../../matrices.h"

/*
 * TODO why is called 63, instead of 64? we use an int64_t and not a 63bit type ...
 *
 * rename it, if it used 64 bit numbers, or if it only used the positive bits, then let it be
 */

int64_t full_sw( sequence * dseq, sequence * qseq, int64_t * hearray ) {
    int64_t h; // current value
    int64_t n; // diagonally previous value
    int64_t e; // value in left cell
    int64_t f; // value in upper cell
    int64_t s;

    int64_t *hep;

    s = 0;
    memset( hearray, 0, 2 * sizeof(int64_t) * (qseq->len) );

#ifdef DBG_COLLECT_MATRIX
        dbg_init_matrix_data_collection( BIT_WIDTH_64, dseq->len, qseq->len );
#endif

    for( size_t j = 0; j < dseq->len; j++ ) {
        hep = hearray;
        f = 0;
        h = 0;

        for( size_t i = 0; i < qseq->len; i++ ) {
            n = *hep;
            e = *(hep + 1);

            h += SCORE_MATRIX_63( dseq->seq[j], qseq->seq[i] );

            if( e > h )
                h = e;
            if( f > h )
                h = f;
            if( h < 0 )
                h = 0;
            if( h > s )
                s = h;

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_64( i, j, h);
#endif

            *hep = h;
            e -= gapE;
            f -= gapE;
            h += -gapO + -gapE;

            if( h > e )
                e = h;
            if( h > f )
                f = h;

            *(hep + 1) = e;
            h = n;
            hep += 2;
        }
    }

#ifdef DBG_COLLECT_MATRIX
        sequence * db_sequences = xmalloc( sizeof( sequence ) );
        db_sequences[0] = *dseq;

        dbg_print_matrices_to_file( BIT_WIDTH_64, "SW", qseq->seq, db_sequences, 1 );
#endif

    return s;
}

