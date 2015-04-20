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

/*
 * Prepares the computation of the CIGAR string.
 *
 * Finding the region for a local alignment is based on the implementation in SWIPE:
 * https://github.com/torognes/swipe/blob/master/align.cc
 */

#include "align.h"

#include <stdlib.h>

#include "../util/util.h"
#include "searcher.h"
#include "../matrices.h"
#include "gap_costs.h"

/*
 * TODO merge finding region and finding directions
 */
region_t find_region_for_local( sequence_t a_seq, sequence_t b_seq ) {
    region_t region;

    size_t size = MAX( a_seq.len, b_seq.len );

    int64_t * HH = xmalloc( size * sizeof(long) );
    int64_t * EE = xmalloc( size * sizeof(long) );

    int64_t score = 0;

    // Forward pass
    for( size_t j = 0; j < a_seq.len; j++ ) {
        HH[j] = 0;
        EE[j] = gapO;
    }

    for( size_t i = 0; i < b_seq.len; i++ ) {
        int64_t h = 0;
        int64_t p = 0;
        int64_t f = gapO;

        for( size_t j = 0; j < a_seq.len; j++ ) {
            f = MAX(f, h + gapO) + gapE;
            EE[j] = MAX(EE[j], HH[j] + gapO) + gapE;

            h = p + SCORE_MATRIX_64( a_seq.seq[j], b_seq.seq[i] );

            if( h < 0 )
                h = 0;
            if( f > h )
                h = f;
            if( EE[j] > h )
                h = EE[j];

            p = HH[j];

            HH[j] = h;

            if( h > score ) {
                score = h;
                region.a_end = j;
                region.b_end = i;
            }
        }
    }

    // Reverse pass
    for( long j = region.b_end; j >= 0; j-- ) { // TODO change long to size_t (be careful because of the down-counting ...)
        HH[j] = -1;
        EE[j] = -1;
    }

    long cost = 0;

    for( long i = region.b_end; i >= 0; i-- ) { // TODO change long to size_t (be careful because of the down-counting ...)
        long h = -1;
        long f = -1;
        long p;
        if( i == region.b_end )
            p = 0;
        else
            p = -1;

        for( long j = region.a_end; j >= 0; j-- ) { // TODO change long to size_t (be careful because of the down-counting ...)
            f = MAX(f, h + gapO) + gapE;
            EE[j] = MAX(EE[j], HH[j] + gapO) + gapE;

            h = p + SCORE_MATRIX_64( a_seq.seq[j], b_seq.seq[i] );

            if( f > h )
                h = f;
            if( EE[j] > h )
                h = EE[j];

            p = HH[j];

            HH[j] = h;

            if( h > cost ) {
                cost = h;
                region.a_begin = j;
                region.b_begin = i;
                if( cost >= score ) {
//                    i = -1; // kind of a break here
//                    j = -1;
                    goto Found;
                    //TODO think about what to use here
                }
            }
        }
    }

    fatal( "Internal error in align function." );

    Found:

    free( EE );
    free( HH );

    return region;
}

region_t init_region_for_global( sequence_t a_seq, sequence_t b_seq ) {
    region_t region;

    region.a_begin = 0;
    region.a_end = a_seq.len - 1;
    region.b_begin = 0;
    region.b_end = b_seq.len - 1;

    return region;
}

static void fill_alignment( p_alignment alignment, region_t region, cigar_p cigar ) {
    alignment->align_q_start = region.a_begin;
    alignment->align_q_end = region.a_end;
    alignment->align_d_start = region.b_begin;
    alignment->align_d_end = region.b_end;
    alignment->alignment = cigar->cigar;
    alignment->alignment_len = cigar->len;
}

void align_sequences( int search_type, p_alignment alignment ) {
    sequence_t a_seq = { alignment->query.seq, alignment->query.len };
    sequence_t b_seq = { alignment->db_seq.seq, alignment->db_seq.len };

    region_t region;
    if( search_type == SMITH_WATERMAN ) {
        region = find_region_for_local( a_seq, b_seq );
    }
    else if( search_type == NEEDLEMAN_WUNSCH ) {
        region = init_region_for_global( a_seq, b_seq );
    }
    else {
        fatal( "\nUnknown search type: %d\n\n", search_type );
    }

    cigar_p cigar = compute_cigar_string( search_type, a_seq, b_seq, region );

    fill_alignment( alignment, region, cigar );

    if( cigar )
        free( cigar );
}
