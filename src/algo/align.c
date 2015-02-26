/*
 * align.c
 *
 *  Created on: Oct 15, 2014
 *      Author: Jakob Frielingsdorf
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
region_t find_region_and_score_for_local( sequence a_seq, sequence b_seq ) {
    region_t region;

    size_t size = MAX( a_seq.len, b_seq.len );

    long * HH = xmalloc( size * sizeof(long) );
    long * EE = xmalloc( size * sizeof(long) );

    long score = 0;

    // Forward pass
    for( size_t j = 0; j < a_seq.len; j++ ) {
        HH[j] = 0;
        EE[j] = -gapO;
    }

    for( size_t i = 0; i < b_seq.len; i++ ) {
        long h = 0;
        long p = 0;
        long f = -gapO;

        for( size_t j = 0; j < a_seq.len; j++ ) {
            f = MAX(f, h - gapO) - gapE;
            EE[j] = MAX(EE[j], HH[j] - gapO) - gapE;

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

    for( long i = region.b_end; i >= 0; i-- ) { // TODO change long to size_t
        long h = -1;
        long f = -1;
        long p;
        if( i == region.b_end )
            p = 0;
        else
            p = -1;

        for( long j = region.a_end; j >= 0; j-- ) { // TODO change long to size_t
            f = MAX(f, h - gapO) - gapE;
            EE[j] = MAX(EE[j], HH[j] - gapO) - gapE;

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

    ffatal( "Internal error in align function." );

    Found:

    free( EE );
    free( HH );

    return region;
}

region_t init_region_for_global( sequence a_seq, sequence b_seq ) {
    region_t region;
// TODO use it, to generalize the functions in cigar.c
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

void align_nw_sellers( p_alignment alignment ) {
    ffatal( "align_nw_sellers: TODO not yet implemented" );

    /*
     * TODO implement this
     */
}

void align_nw( p_alignment alignment ) {
    sequence a_seq = { alignment->query.seq, alignment->query.len };
    sequence b_seq = { alignment->db_seq.seq, alignment->db_seq.len };

    region_t region = init_region_for_global( a_seq, b_seq );

    cigar_p cigar = compute_cigar_for_nw( a_seq, b_seq );

    fill_alignment( alignment, region, cigar );

    free_cigar( cigar );
}

void align_sw( p_alignment alignment ) {
    sequence a_seq = { alignment->query.seq, alignment->query.len };
    sequence b_seq = { alignment->db_seq.seq, alignment->db_seq.len };

    region_t region = find_region_and_score_for_local( a_seq, b_seq );

    cigar_p cigar = compute_cigar_for_sw( a_seq, b_seq, region );

    fill_alignment( alignment, region, cigar );

    free_cigar( cigar );
}
