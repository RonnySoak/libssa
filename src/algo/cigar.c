/*
 * cigar->c
 *
 *  Created on: Oct 16, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "align.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../util/util.h"
#include "../matrices.h"
#include "gap_costs.h"

#define CIGAR_ALLOC_STEP_SIZE 64;

/*
 * Const here, instead of a define, since we want to have them in size of 1 byte.
 * XXX maybe a define works as well
 */
const uint8_t MASK_GAP_UP = 1;
const uint8_t MASK_GAP_LEFT = 2;
const uint8_t MASK_GAP_EXT_UP = 4;
const uint8_t MASK_GAP_EXT_LEFT = 8;

static uint8_t * compute_directions_for_nw( sequence a_seq, sequence b_seq ) {
    uint8_t * directions = xmalloc( a_seq.len * b_seq.len );
    /*
     * Holds in the first column the scores of the previous column
     * and in the second column the gap-values of the previous column
     */
    int64_t * hearray = xmalloc( 2 * a_seq.len * sizeof(int64_t) );

    int64_t h; // current value
    int64_t n; // diagonally previous value
    int64_t e; // value in left cell
    int64_t f; // value in upper cell
    int64_t *hep;

    memset( directions, 0, a_seq.len * b_seq.len );

    for( size_t i = 0; i < a_seq.len; i++ ) {
        hearray[2 * i] = -gapO + (i + 1) * -gapE; // H (N)    scores in previous column
        hearray[2 * i + 1] = hearray[2 * i]; // E    gap values in previous column
    }

    for( size_t j = 0; j < b_seq.len; j++ ) {
        hep = hearray;
        f = -gapO + (j + 1) * -gapE;          	// value in first upper cell
        h = (j == 0) ? 0 : (-gapO + j * -gapE); // value in first cell of line

        for( size_t i = 0; i < a_seq.len; i++ ) {
            size_t index = a_seq.len * j + i;

            n = *hep;
            e = *(hep + 1);
            h += SCORE_MATRIX_64( b_seq.seq[j], a_seq.seq[i] );

            // test for gap opening
            if( f > h ) {
                directions[index] |= MASK_GAP_UP;
                h = f;
            }
            if( e > h ) {
                h = e;
                directions[index] |= MASK_GAP_LEFT;
            }

            *hep = h;

            h += -gapO + -gapE;

            // test for gap extensions
            e += -gapE;
            f += -gapE;

            if( f > h ) {
                directions[index] |= MASK_GAP_EXT_UP;
            }
            else {
                f = h;
            }

            if( e > h ) {
                directions[index] |= MASK_GAP_EXT_LEFT;
            }
            else {
                e = h;
            }

            // next round
            *(hep + 1) = e;
            h = n;
            hep += 2;
        }
    }

    free( hearray );

    return directions;
}

static uint8_t * compute_directions_for_sw( sequence a_seq, sequence b_seq ) {
    uint8_t * directions = xmalloc( a_seq.len * b_seq.len );
    /*
     * Holds in the first column the scores of the previous column
     * and in the second column the gap-values of the previous column
     */
    int64_t * hearray = xmalloc( 2 * a_seq.len * sizeof(int64_t) );

    int64_t h; // current value
    int64_t n; // diagonally previous value
    int64_t e; // value in left cell
    int64_t f; // value in upper cell
    int64_t *hep;

    memset( directions, 0, a_seq.len * b_seq.len );
    memset( hearray, 0, 2 * a_seq.len * sizeof(int64_t) );

    for( size_t j = 0; j < b_seq.len; j++ ) {
        hep = hearray;
        f = 0;   // value in first upper cell
        h = 0;   // value in first cell of line

        for( size_t i = 0; i < a_seq.len; i++ ) {
            size_t index = a_seq.len * j + i;

            n = *hep;
            e = *(hep + 1);
            h += SCORE_MATRIX_64( b_seq.seq[j], a_seq.seq[i] );

            // test for gap opening
            if( f > h ) {
                directions[index] |= MASK_GAP_UP;
                h = f;
            }
            if( e > h ) {
                h = e;
                directions[index] |= MASK_GAP_LEFT;
            }
            if( h < 0 ) {
                h = 0;
            }

            *hep = h;

            h += -gapO + -gapE;

            // test for gap extensions
            e += -gapE;
            f += -gapE;

            if( f > h ) {
                directions[index] |= MASK_GAP_EXT_UP;
            }
            else {
                f = h;
            }

            if( e > h ) {
                directions[index] |= MASK_GAP_EXT_LEFT;
            }
            else {
                e = h;
            }

            // next round
            *(hep + 1) = e;
            h = n;
            hep += 2;
        }
    }

    free( hearray );

    return directions;
}

static inline void check_allocated_size( cigar_p cigar ) {
    while( cigar->len >= cigar->allocated_size ) {
        cigar->allocated_size += CIGAR_ALLOC_STEP_SIZE;
        cigar->cigar = xrealloc( cigar->cigar, cigar->allocated_size * sizeof(char) );
    }
}

// TODO implement the cigar creation, so it does not need to write number reversed ...
char * strrev( char *str ) {
    char *p1, *p2;

    if( !str || !*str )
        return str;
    for( p1 = str, p2 = str + strlen( str ) - 1; p2 > p1; ++p1, --p2 ) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

static inline void add_to_cigar( char op, size_t op_count, cigar_p cigar ) {
    if( op_count == 0 ) {
        return;
    }
    cigar->cigar[cigar->len++] = op;

    check_allocated_size( cigar ); // TODO call this only once

    if( op_count > 1 ) {
        uint64_t insert_pos = cigar->len;

        char counter_buf[25];
        size_t len = sprintf( counter_buf, "%ld", op_count );
        cigar->len += len;

        check_allocated_size( cigar );

        // TODO replace strrev call by more intelligent code, not using it
        strncpy( cigar->cigar + insert_pos, strrev( counter_buf ), len );
    }
}

cigar_p reverse_cigar( cigar_p rev_cigar ) {
    /*
     * TODO might be better to use a realloc and reverse the cigar string in
     * place. This could possibly save a lot of memory movements.
     */
    cigar_p cigar = xmalloc( sizeof(cigar_t) );
    cigar->len = rev_cigar->len;
    cigar->allocated_size = cigar->len + 1;

    cigar->cigar = xmalloc( cigar->allocated_size * sizeof(char) );

    size_t cur_pos = 0;
    while( rev_cigar->len > 0 ) {
        cigar->cigar[cur_pos] = rev_cigar->cigar[--rev_cigar->len];

        cur_pos++;
    }
    cigar->cigar[cur_pos] = 0;

    return cigar;
}

cigar_p compute_cigar_for_nw( sequence a_seq, sequence b_seq ) {
    /*
     * cigar operation characters:
     *
     * 'M': alignment match (can be a sequence match or mismatch
     * 'I': insertion to the reference
     * 'D': deletion from the reference
     * 'N': skipped region from the reference
     * 'S': soft clipping (clipped sequences present in SEQ)
     * 'H': hard clipping (clipped sequences NOT present in SEQ)
     * 'P': padding (silent deletion from padded reference)
     * '=': sequence match
     * 'X': sequence mismatch
     *
     * TODO at the moment are only 'M', 'I' and 'D' implemented.
     *  Do we need the rest as well?
     */

    cigar_p rev_cigar = xmalloc( sizeof(cigar_t) );
    rev_cigar->allocated_size = CIGAR_ALLOC_STEP_SIZE;
    rev_cigar->len = 0;
    rev_cigar->cigar = xmalloc( rev_cigar->allocated_size * sizeof(char) );

    uint8_t * directions = compute_directions_for_nw( a_seq, b_seq );

    size_t i = a_seq.len;
    size_t j = b_seq.len;

    char prev_op = 0, op = 0;
    size_t op_count = 0;
    while( (i > 0) && (j > 0) ) {
        uint8_t d = directions[a_seq.len * (j - 1) + (i - 1)];

        if( (d & MASK_GAP_LEFT) || (d & MASK_GAP_EXT_LEFT) ) {
            j--;
            op = 'I';
        }
        else if( (d & MASK_GAP_UP) || (d & MASK_GAP_EXT_UP) ) {
            i--;
            op = 'D';
        }
        else {
            i--;
            j--;
            op = 'M';
        }

        if( op == prev_op ) {
            op_count++;
        }
        else {
            add_to_cigar( prev_op, op_count, rev_cigar );

            prev_op = op;
            op_count = 1;
        }
    }
    add_to_cigar( prev_op, op_count, rev_cigar );

    if( i > 0 ) {
        op = 'D';
        op_count = i;
    }
    else {
        op = 'I';
        op_count = j;
    }
    add_to_cigar( op, op_count, rev_cigar );

    cigar_p result = reverse_cigar( rev_cigar );

    free( rev_cigar->cigar );
    free( rev_cigar );
    free( directions );

    return result;
}

cigar_p compute_cigar_for_sw( sequence a_seq, sequence b_seq, region_t region ) {
    /*
     * cigar operation characters:
     *
     * 'M': alignment match (can be a sequence match or mismatch
     * 'I': insertion to the reference
     * 'D': deletion from the reference
     * 'N': skipped region from the reference
     * 'S': soft clipping (clipped sequences present in SEQ)
     * 'H': hard clipping (clipped sequences NOT present in SEQ)
     * 'P': padding (silent deletion from padded reference)
     * '=': sequence match
     * 'X': sequence mismatch
     *
     * TODO at the moment are only 'M', 'I' and 'D' implemented.
     *  Do we need the rest as well?
     */

    cigar_p rev_cigar = xmalloc( sizeof(cigar_t) );
    rev_cigar->allocated_size = CIGAR_ALLOC_STEP_SIZE;
    rev_cigar->len = 0;
    rev_cigar->cigar = xmalloc( rev_cigar->allocated_size * sizeof(char) );

    uint8_t * directions = compute_directions_for_sw( a_seq, b_seq );

    size_t i = region.a_end;
    size_t j = region.b_end;

    char prev_op = 0, op = 0;
    size_t op_count = 0;

    while( (i + 1 > 0) && (j + 1 > 0) && (i >= region.a_begin) && (j >= region.b_begin) ) {
        uint8_t d = directions[a_seq.len * j + i];

        if( (d & MASK_GAP_LEFT) || (d & MASK_GAP_EXT_LEFT) ) {
            j--;
            op = 'I';
        }
        else if( (d & MASK_GAP_UP) || (d & MASK_GAP_EXT_UP) ) {
            i--;
            op = 'D';
        }
        else {
            i--;
            j--;
            op = 'M';
        }

        if( op == prev_op ) {
            op_count++;
        }
        else {
            add_to_cigar( prev_op, op_count, rev_cigar );

            prev_op = op;
            op_count = 1;
        }
    }
    add_to_cigar( prev_op, op_count, rev_cigar );

    cigar_p result = reverse_cigar( rev_cigar );

    free( rev_cigar->cigar );
    free( rev_cigar );
    free( directions );

    return result;
}

void free_cigar( cigar_p cigar ) {
    if( cigar ) {
        free( cigar );
    }
}
