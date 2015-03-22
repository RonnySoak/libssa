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

#ifdef SEARCH_8_BIT
#include "../8/search_8.h"
#include "../8/search_8_util.h"
#else
#include "../16/search_16.h"
#include "../16/search_16_util.h"
#endif

#include <limits.h>
#include <string.h>

#include "../../util/util.h"

#ifdef __AVX2__

#define _mmxxx_and_si _mm256_and_si256
#define _mmxxx_or_si _mm256_or_si256
#define _mmxxx_setzero_si _mm256_setzero_si256
#define _mmxxx_movemask_epi8 _mm256_movemask_epi8

#else // SSE2

#define _mmxxx_and_si _mm_and_si128
#define _mmxxx_or_si _mm_or_si128
#define _mmxxx_setzero_si _mm_setzero_si128
#define _mmxxx_movemask_epi8 _mm_movemask_epi8

#endif

#ifdef SEARCH_8_BIT

#define CHANNELS CHANNELS_8_BIT
#define BIT_WIDTH BIT_WIDTH_8
#define CDEPTH CDEPTH_8_BIT

#define I_MIN INT8_MIN
#define I_MAX INT8_MAX
#define UI_MAX UINT8_MAX

typedef p_s8info p_sYYinfo;
typedef int8_t intYY_t;

#define move_db_sequence_window_YY move_db_sequence_window_8

#ifdef __AVX2__

#define _mmxxx_adds_epiYY _mm256_adds_epi8
#define _mmxxx_subs_epiYY _mm256_subs_epi8
#define _mmxxx_subs_epuYY _mm256_subs_epu8
#define _mmxxx_min_epiYY _mm256_min_epi8
#define _mmxxx_max_epiYY _mm256_max_epi8
#define _mmxxx_set1_epiYY _mm256_set1_epi8
#define _mmxxx_cmpeq_epiYY _mm256_cmpeq_epi8
#define _mmxxx_cmpgt_epiYY _mm256_cmpgt_epi8

#define search_YY_XXX_nw search_8_avx2_nw
#define dprofile_fill_YY_xxx dprofile_fill_8_avx2
#define dbg_add_matrix_data_xxx_YY_nw dbg_add_matrix_data_256_8_nw

#else // SSE4.1

#define _mmxxx_adds_epiYY _mm_adds_epi8
#define _mmxxx_subs_epiYY _mm_subs_epi8
#define _mmxxx_subs_epuYY _mm_subs_epu8
#define _mmxxx_min_epiYY _mm_min_epi8
#define _mmxxx_max_epiYY _mm_max_epi8
#define _mmxxx_set1_epiYY _mm_set1_epi8
#define _mmxxx_cmpeq_epiYY _mm_cmpeq_epi8
#define _mmxxx_cmpgt_epiYY _mm_cmpgt_epi8

#define search_YY_XXX_nw search_8_sse41_nw
#define dprofile_fill_YY_xxx dprofile_fill_8_sse41
#define dbg_add_matrix_data_xxx_YY_nw dbg_add_matrix_data_128_8_nw

#endif

#else // search 16 bit

#define BIT_WIDTH BIT_WIDTH_16
#define CHANNELS CHANNELS_16_BIT
#define CDEPTH CDEPTH_16_BIT

#define I_MIN INT16_MIN
#define I_MAX INT16_MAX
#define UI_MAX UINT16_MAX

typedef p_s16info p_sYYinfo;
typedef int16_t intYY_t;

#define move_db_sequence_window_YY move_db_sequence_window_16

#ifdef __AVX2__

#define _mmxxx_adds_epiYY _mm256_adds_epi16
#define _mmxxx_subs_epiYY _mm256_subs_epi16
#define _mmxxx_subs_epuYY _mm256_subs_epu16
#define _mmxxx_min_epiYY _mm256_min_epi16
#define _mmxxx_max_epiYY _mm256_max_epi16
#define _mmxxx_set1_epiYY _mm256_set1_epi16
#define _mmxxx_cmpeq_epiYY _mm256_cmpeq_epi16
#define _mmxxx_cmpgt_epiYY _mm256_cmpgt_epi16

#define search_YY_XXX_nw search_16_avx2_nw
#define dprofile_fill_YY_xxx dprofile_fill_16_avx2
#define dbg_add_matrix_data_xxx_YY dbg_add_matrix_data_256_16

#else // SSE2

#define _mmxxx_adds_epiYY _mm_adds_epi16
#define _mmxxx_subs_epiYY _mm_subs_epi16
#define _mmxxx_subs_epuYY _mm_subs_epu16
#define _mmxxx_min_epiYY _mm_min_epi16
#define _mmxxx_max_epiYY _mm_max_epi16
#define _mmxxx_set1_epiYY _mm_set1_epi16
#define _mmxxx_cmpeq_epiYY _mm_cmpeq_epi16
#define _mmxxx_cmpgt_epiYY _mm_cmpgt_epi16

#define search_YY_XXX_nw search_16_sse2_nw
#define dprofile_fill_YY_xxx dprofile_fill_16_sse2
#define dbg_add_matrix_data_xxx_YY dbg_add_matrix_data_128_16
#endif

#endif // end search

/*
 Using 16-bit signed values, from -32768 to +32767.
 match: positive
 mismatch: negative
 gap penalties: positive (open, extend)
 optimal global alignment (NW)
 maximize score
 */
#ifdef DBG_COLLECT_MATRIX
static int d_idx;
#endif

/*
 * All parameters are vectors of the type __mxxxi
 *
 * H: diagonal alignment score
 * N: alignment score of the current cell, saved for computing the cell in the next row
 * E: gaps in the query sequence
 * F: gaps in the database sequences
 * V: substitution scores from the db profile
 * QR: gap open extend costs
 * R: gap extension costs
 * H_MIN: min score of this block
 * H_MAX: max score of this block
 */
#define ALIGNCORE(H, N, E, F, V, QR, R, H_MIN, H_MAX)                          \
 H = _mmxxx_adds_epiYY(H, V);         /* add value of scoring profile */       \
 H = _mmxxx_max_epiYY(H, F);          /* max(H, F) */                          \
 H = _mmxxx_max_epiYY(H, E);          /* max(H, E) */                          \
 H_MIN = _mmxxx_min_epiYY(H_MIN, H);                                           \
 H_MAX = _mmxxx_max_epiYY(H_MAX, H);                                           \
 N = H;                               /* save H in HE-array */                 \
 H = _mmxxx_subs_epiYY(H, QR);        /* subtract gap open-extend */           \
 F = _mmxxx_subs_epiYY(F, R);         /* subtract gap extend */                \
 F = _mmxxx_max_epiYY(F, H);          /* test for gap extension, or opening */ \
 E = _mmxxx_subs_epiYY(E, R);         /* subtract gap extend */                \
 E = _mmxxx_max_epiYY(E, H);          /* test for gap extension, or opening */

static void aligncolumns_first( __mxxxi * Sm, __mxxxi * hep, __mxxxi ** qp, __mxxxi gap_open_extend, __mxxxi gap_extend,
        __mxxxi h0, __mxxxi h1, __mxxxi h2, __mxxxi h3, __mxxxi f0, __mxxxi f1, __mxxxi f2, __mxxxi f3,
        __mxxxi * _h_min, __mxxxi * _h_max, __mxxxi M, size_t ql ) {
    __mxxxi h4, h5, h6, h7, h8, E;
    __mxxxi * vp;
    /*
     * We set h_min and h_max to zero, to prevent a reuse of the previous score
     * in the same channel.
     */
    *_h_min = _mmxxx_setzero_si();
    *_h_max = _mmxxx_setzero_si();

    /* make masked versions of QR, R and E0 */
    __mxxxi M_gap_open_extend = _mmxxx_and_si( M, gap_open_extend );
    __mxxxi M_gap_extend = _mmxxx_and_si( M, gap_extend );

    __mxxxi M_gap_extension = M_gap_open_extend;

    f0 = _mmxxx_subs_epiYY( f0, gap_open_extend );
    f1 = _mmxxx_subs_epiYY( f1, gap_open_extend );
    f2 = _mmxxx_subs_epiYY( f2, gap_open_extend );
    f3 = _mmxxx_subs_epiYY( f3, gap_open_extend );

    for( size_t i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        /*
         * Initialize selected h and e values for next/this round.
         * First zero those cells where a new sequence starts
         * by using an unsigned saturated subtraction of a huge value to
         * set it to zero.
         * Then use signed subtraction to obtain the correct value.
         *
         * M_gap_open_extend is initialized with the gap open extend costs and
         * each round the extend costs are subtracted.
         */
        h4 = _mmxxx_subs_epuYY( h4, M );
        h4 = _mmxxx_subs_epiYY( h4, M_gap_extension );

        E = _mmxxx_subs_epuYY( E, M );
        E = _mmxxx_subs_epiYY( E, M_gap_extension );
        E = _mmxxx_subs_epiYY( E, M_gap_open_extend );

        M_gap_extension = _mmxxx_adds_epiYY( M_gap_extension, M_gap_extend );

        ALIGNCORE( h0, h5, E, f0, vp[0], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h1, h6, E, f1, vp[1], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h2, h7, E, f2, vp[2], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h3, h8, E, f3, vp[3], gap_open_extend, gap_extend, *_h_min, *_h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_xxx_YY( i, d_idx + 0, h5 );
        dbg_add_matrix_data_xxx_YY( i, d_idx + 1, h6 );
        dbg_add_matrix_data_xxx_YY( i, d_idx + 2, h7 );
        dbg_add_matrix_data_xxx_YY( i, d_idx + 3, h8 );
#endif

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;
    }

    Sm[0] = h1;
    Sm[1] = h2;
    Sm[2] = h3;
    Sm[3] = hep[2 * (ql - 1) + 0];
}

static void aligncolumns_rest( __mxxxi * Sm, __mxxxi * hep, __mxxxi ** qp, __mxxxi gap_open_extend, __mxxxi gap_extend,
        __mxxxi h0, __mxxxi h1, __mxxxi h2, __mxxxi h3, __mxxxi f0, __mxxxi f1, __mxxxi f2, __mxxxi f3,
        __mxxxi * _h_min, __mxxxi * _h_max, size_t ql ) {
    __mxxxi h4, h5, h6, h7, h8, E;
    __mxxxi * vp;

    f0 = _mmxxx_subs_epiYY( f0, gap_open_extend );
    f1 = _mmxxx_subs_epiYY( f1, gap_open_extend );
    f2 = _mmxxx_subs_epiYY( f2, gap_open_extend );
    f3 = _mmxxx_subs_epiYY( f3, gap_open_extend );

    for( size_t i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, E, f0, vp[0], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h1, h6, E, f1, vp[1], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h2, h7, E, f2, vp[2], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h3, h8, E, f3, vp[3], gap_open_extend, gap_extend, *_h_min, *_h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_xxx_YY( i, d_idx + 0, h5 );
        dbg_add_matrix_data_xxx_YY( i, d_idx + 1, h6 );
        dbg_add_matrix_data_xxx_YY( i, d_idx + 2, h7 );
        dbg_add_matrix_data_xxx_YY( i, d_idx + 3, h8 );
#endif

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;
    }

    Sm[0] = h1;
    Sm[1] = h2;
    Sm[2] = h3;
    Sm[3] = hep[2 * (ql - 1) + 0];
}

void search_YY_XXX_nw( p_sYYinfo s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t q_id ) {

#ifdef DBG_COLLECT_MATRIX
    size_t maxdlen =  0;
    for( int i = 0; i < chunk->fill_pointer; ++i ) {
        if( maxdlen < chunk->seq[i]->seq.len ) {
            maxdlen = chunk->seq[i]->seq.len;
        }
    }

    dbg_init_matrix_data_collection( BIT_WIDTH, maxdlen + CDEPTH, s->maxqlen );

    d_idx = 0;
#endif

    size_t qlen = s->queries[q_id]->q_len;

    __mxxxi gap_open_extend, gap_extend;

    __mxxxi * hep;

    uint8_t * d_begin[CHANNELS];
    uint8_t * d_end[CHANNELS];
    size_t d_length[CHANNELS];
    p_sdb_sequence d_seq_ptr[CHANNELS];

    union {
        __mxxxi v[CDEPTH];
        intYY_t a[CDEPTH * CHANNELS];
    } S;
    union {
        __mxxxi v;
        intYY_t a[CHANNELS];
    } M;
    union {
        __mxxxi v;
        intYY_t a[CHANNELS];
    } overflow;

    overflow.v = _mmxxx_setzero_si();

    uint8_t dseq_search_window[CDEPTH * CHANNELS];

    size_t next_id = 0;
    size_t done = 0;

    gap_open_extend = _mmxxx_set1_epiYY( s->penalty_gap_open + s->penalty_gap_extension );
    gap_extend = _mmxxx_set1_epiYY( s->penalty_gap_extension );

    hep = s->hearray;

    for( int c = 0; c < CHANNELS; c++ ) {
        d_begin[c] = 0;
        d_end[c] = d_begin[c];
        d_length[c] = 0;
        d_seq_ptr[c] = 0;
    }

    __mxxxi score_min = _mmxxx_set1_epiYY( I_MIN + s->penalty_gap_open + s->penalty_gap_extension -1 ); // TODO why + Q + R ???
    __mxxxi score_max = _mmxxx_set1_epiYY( I_MAX );

    __mxxxi H0 = _mmxxx_setzero_si();
    __mxxxi H1 = _mmxxx_setzero_si();
    __mxxxi H2 = _mmxxx_setzero_si();
    __mxxxi H3 = _mmxxx_setzero_si();

    __mxxxi F0 = _mmxxx_setzero_si();
    __mxxxi F1 = _mmxxx_setzero_si();
    __mxxxi F2 = _mmxxx_setzero_si();
    __mxxxi F3 = _mmxxx_setzero_si();

    __mxxxi h_min;
    __mxxxi h_max;

    int change_sequences = 1;
    while( 1 ) {
        if( !change_sequences ) {
            /* fill all channels with symbols from the database sequences */

            for( int c = 0; c < CHANNELS; c++ ) {
                if( d_seq_ptr[c] )
                    change_sequences |= move_db_sequence_window_YY( c, d_begin, d_end, dseq_search_window );
            }

            dprofile_fill_YY_xxx( s->dprofile, dseq_search_window );

            aligncolumns_rest( S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, H0, H1, H2, H3, F0, F1,
                    F2, F3, &h_min, &h_max, qlen );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            change_sequences = 0;

            M.v = _mmxxx_setzero_si();
            for( int c = 0; c < CHANNELS; c++ ) {
                if( !overflow.a[c] && (d_begin[c] < d_end[c]) ) {
                    /* the sequence in this channel is not finished yet */

                    change_sequences |= move_db_sequence_window_YY( c, d_begin, d_end, dseq_search_window );
                }
                else {
                    /* sequence in channel c ended. change of sequence */

                    M.a[c] = UI_MAX;

                    if( d_seq_ptr[c] ) {
                        /* save score */

                        long z = (d_length[c] + 3) % 4;
                        long score = S.a[z * CHANNELS + c];

                        if( !overflow.a[c] && (score > I_MIN) && (score < I_MAX) ) {
                            add_to_minheap( heap, q_id, d_seq_ptr[c], score );
                        }
                        else {
                            overflow_chunk->seq[overflow_chunk->fill_pointer++] = d_seq_ptr[c];
                        }

                        done++;
                    }

                    if( next_id < chunk->fill_pointer ) {
                        /* get next sequence with length>0 */

                        d_seq_ptr[c] = chunk->seq[next_id++];

                        d_length[c] = d_seq_ptr[c]->seq.len;
                        d_begin[c] = (unsigned char*) d_seq_ptr[c]->seq.seq;
                        d_end[c] = (unsigned char*) d_seq_ptr[c]->seq.seq + d_seq_ptr[c]->seq.len;

                        ((intYY_t*) &H0)[c] = 0;
                        ((intYY_t*) &H1)[c] = -s->penalty_gap_open - 1 * s->penalty_gap_extension;
                        ((intYY_t*) &H2)[c] = -s->penalty_gap_open - 2 * s->penalty_gap_extension;
                        ((intYY_t*) &H3)[c] = -s->penalty_gap_open - 3 * s->penalty_gap_extension;

                        ((intYY_t*) &F0)[c] = -s->penalty_gap_open - 1 * s->penalty_gap_extension;
                        ((intYY_t*) &F1)[c] = -s->penalty_gap_open - 2 * s->penalty_gap_extension;
                        ((intYY_t*) &F2)[c] = -s->penalty_gap_open - 3 * s->penalty_gap_extension;
                        ((intYY_t*) &F3)[c] = -s->penalty_gap_open - 4 * s->penalty_gap_extension;

                        change_sequences |= move_db_sequence_window_YY( c, d_begin, d_end, dseq_search_window );
                    }
                    else {
                        /* no more sequences, empty channel */

                        d_seq_ptr[c] = 0;
                        d_begin[c] = 0;
                        d_end[c] = d_begin[c];
                        d_length[c] = 0;
                        for( int j = 0; j < CDEPTH; j++ )
                            dseq_search_window[CHANNELS * j + c] = 0;
                    }
                }
            }

            if( done == chunk->fill_pointer )
                break;

            dprofile_fill_YY_xxx( s->dprofile, dseq_search_window );

            aligncolumns_first( S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, H0, H1, H2, H3, F0,
                    F1, F2, F3, &h_min, &h_max, M.v, qlen );
        }

        /*
         * An overflow enforces a sequence change in the corresponding channel,
         * since this sequence has to be re-aligned anyway.
         */
        overflow.v = _mmxxx_cmpgt_epiYY( score_min, h_min );
        overflow.v = _mmxxx_or_si( _mmxxx_cmpeq_epiYY( h_max, score_max ), overflow.v );
        change_sequences |= _mmxxx_movemask_epi8( overflow.v );

#ifdef DBG_COLLECT_MATRIX
        d_idx += 4;
#endif

        /*
         * Before calling it again, we need to add CDEPTH * gap_extend to f0, to move it to the new column.
         */
        F0 = _mmxxx_subs_epiYY( F3, gap_extend );
        F1 = _mmxxx_subs_epiYY( F0, gap_extend );
        F2 = _mmxxx_subs_epiYY( F1, gap_extend );
        F3 = _mmxxx_subs_epiYY( F2, gap_extend );

        H0 = _mmxxx_subs_epiYY( H3, gap_extend );
        H1 = _mmxxx_subs_epiYY( H0, gap_extend );
        H2 = _mmxxx_subs_epiYY( H1, gap_extend );
        H3 = _mmxxx_subs_epiYY( H2, gap_extend );
    }

#ifdef DBG_COLLECT_MATRIX
    sequence_t * db_sequences = xmalloc( sizeof( sequence_t ) * done );

    for (int i = 0; i < done; ++i) {
        db_sequences[i] = chunk->seq[i]->seq;
    }

    dbg_print_matrices_to_file( BIT_WIDTH, "NW", s->queries[q_id]->seq, db_sequences, done );

    free( db_sequences );
#endif
}
