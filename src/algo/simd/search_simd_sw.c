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

/*
 * use the range from -32768 - 0 - to 32767
 *
 * init with int16_min and add int16_max to max score
 *
 * add -32768 twice, to reset to zero (modify masking values)
 *
 * a biased Version adds more instructions in ALIGNCORE, than the other version
 */
#ifdef __AVX2__

#define _mmxxx_or_si _mm256_or_si256
#define _mmxxx_setzero_si _mm256_setzero_si256
#define _mmxxx_movemask_epi8 _mm256_movemask_epi8

#else // SSE4.1

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
#define _mmxxx_max_epiYY _mm256_max_epi8
#define _mmxxx_set1_epiYY _mm256_set1_epi8
#define _mmxxx_cmpeq_epiYY _mm256_cmpeq_epi8

#define search_YY_XXX_sw search_8_avx2_sw
#define dprofile_fill_YY_xxx dprofile_fill_8_avx2
#define dbg_add_matrix_data_xxx_YY_sw dbg_add_matrix_data_256_8_sw

#else // SSE4.1

#define _mmxxx_adds_epiYY _mm_adds_epi8
#define _mmxxx_subs_epiYY _mm_subs_epi8
#define _mmxxx_max_epiYY _mm_max_epi8
#define _mmxxx_set1_epiYY _mm_set1_epi8
#define _mmxxx_cmpeq_epiYY _mm_cmpeq_epi8

#define search_YY_XXX_sw search_8_sse41_sw
#define dprofile_fill_YY_xxx dprofile_fill_8_sse41
#define dbg_add_matrix_data_xxx_YY_sw dbg_add_matrix_data_128_8_sw

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
#define _mmxxx_max_epiYY _mm256_max_epi16
#define _mmxxx_set1_epiYY _mm256_set1_epi16
#define _mmxxx_cmpeq_epiYY _mm256_cmpeq_epi16

#define search_YY_XXX_sw search_16_avx2_sw
#define dprofile_fill_YY_xxx dprofile_fill_16_avx2
#define dbg_add_matrix_data_xxx_YY_sw dbg_add_matrix_data_256_16_sw

#else // SSE2

#define _mmxxx_adds_epiYY _mm_adds_epi16
#define _mmxxx_subs_epiYY _mm_subs_epi16
#define _mmxxx_max_epiYY _mm_max_epi16
#define _mmxxx_set1_epiYY _mm_set1_epi16
#define _mmxxx_cmpeq_epiYY _mm_cmpeq_epi16

#define search_YY_XXX_sw search_16_sse2_sw
#define dprofile_fill_YY_xxx dprofile_fill_16_sse2
#define dbg_add_matrix_data_xxx_YY_sw dbg_add_matrix_data_128_16_sw

#endif

#endif // end search

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
 * S: max score of this alignment
 */
#define ALIGNCORE(H, N, E, F, V, QR, R, S )                                	   \
 H = _mmxxx_adds_epiYY(H, V);         /* add value of scoring profile */       \
 H = _mmxxx_max_epiYY(H, F);          /* max(H, F) */                          \
 H = _mmxxx_max_epiYY(H, E);          /* max(H, E) */                          \
 S = _mmxxx_max_epiYY(H, S);          /* save max score */                     \
 N = H;                               /* save H in HE-array */                 \
 H = _mmxxx_subs_epiYY(H, QR);        /* subtract gap open-extend */           \
 F = _mmxxx_subs_epiYY(F, R);         /* subtract gap extend */                \
 F = _mmxxx_max_epiYY(F, H);          /* test for gap extension, or opening */ \
 E = _mmxxx_subs_epiYY(E, R);         /* subtract gap extend */                \
 E = _mmxxx_max_epiYY(E, H);          /* test for gap extension, or opening */

static void aligncolumns_first( __mxxxi * Sm, __mxxxi * hep, __mxxxi ** qp, __mxxxi gap_open_extend, __mxxxi gap_extend,
        __mxxxi M, size_t ql ) {
    __mxxxi h4, h5, h6, h7, h8, f0, f1, f2, f3, E;
    __mxxxi * vp;

    __mxxxi VECTOR_INT_MIN = _mmxxx_set1_epiYY( I_MIN );

    __mxxxi h0, h1, h2, h3;

    h0 = h1 = h2 = h3 = VECTOR_INT_MIN;
    f0 = f1 = f2 = f3 = VECTOR_INT_MIN;

    for( size_t i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        /*
         * h4 and E are set to INT16_MIN for database sequences, that begin here
         *
         * To set both to INT16_MIN, we simply subtract INT16_MIN twice.
         *
         * Mm is set to INT16_MIN on all channels, where new database sequences begin,
         * the other channels are not affected.
         */
        h4 = _mmxxx_adds_epiYY( h4, M );
        h4 = _mmxxx_adds_epiYY( h4, M );

        E = hep[2 * i + 1];
        E = _mmxxx_adds_epiYY( E, M );
        E = _mmxxx_adds_epiYY( E, M );

        ALIGNCORE( h0, h5, E, f0, vp[0], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h1, h6, E, f1, vp[1], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h2, h7, E, f2, vp[2], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h3, h8, E, f3, vp[3], gap_open_extend, gap_extend, *Sm );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_xxx_YY_sw( i, d_idx + 0, h5 );
        dbg_add_matrix_data_xxx_YY_sw( i, d_idx + 1, h6 );
        dbg_add_matrix_data_xxx_YY_sw( i, d_idx + 2, h7 );
        dbg_add_matrix_data_xxx_YY_sw( i, d_idx + 3, h8 );
#endif

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;
    }
}

static void aligncolumns_rest( __mxxxi * Sm, __mxxxi * hep, __mxxxi ** qp, __mxxxi gap_open_extend, __mxxxi gap_extend,
        size_t ql ) {
    __mxxxi h4, h5, h6, h7, h8, f0, f1, f2, f3, E;
    __mxxxi * vp;

    __mxxxi VECTOR_INT_MIN = _mmxxx_set1_epiYY( I_MIN );

    __mxxxi h0, h1, h2, h3;

    h0 = h1 = h2 = h3 = VECTOR_INT_MIN;
    f0 = f1 = f2 = f3 = VECTOR_INT_MIN;

    for( size_t i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, E, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h1, E, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h2, E, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h3, E, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_xxx_YY_sw( i, d_idx + 0, h5 );
        dbg_add_matrix_data_xxx_YY_sw( i, d_idx + 1, h6 );
        dbg_add_matrix_data_xxx_YY_sw( i, d_idx + 2, h7 );
        dbg_add_matrix_data_xxx_YY_sw( i, d_idx + 3, h8 );
#endif

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;
    }
}

void search_YY_XXX_sw( p_sYYinfo s, p_db_chunk chunk, p_minheap heap, p_db_chunk overflow_chunk, uint8_t q_id ) {

#ifdef DBG_COLLECT_MATRIX
    size_t maxdlen = 0;
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
    p_sdb_sequence d_seq_ptr[CHANNELS];

    union {
        __mxxxi v;
        intYY_t a[CHANNELS];
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
        d_seq_ptr[c] = 0;
    }

    __mxxxi score_max = _mmxxx_set1_epiYY( I_MAX );

    int no_sequences_ended = 0;
    while( 1 ) {
        if( no_sequences_ended ) {
            /* fill all channels with symbols from the database sequences */

            for( int c = 0; c < CHANNELS; c++ ) {
                if( d_seq_ptr[c] )
                    no_sequences_ended &= move_db_sequence_window_YY( c, d_begin, d_end, dseq_search_window );
            }

            dprofile_fill_YY_xxx( s->dprofile, dseq_search_window );

            aligncolumns_rest( &S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, qlen );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            no_sequences_ended = 1;

            M.v = _mmxxx_setzero_si();
            for( int c = 0; c < CHANNELS; c++ ) {
                if( !overflow.a[c] && (d_begin[c] < d_end[c]) ) {
                    /* the sequence in this channel is not finished yet */

                    no_sequences_ended &= move_db_sequence_window_YY( c, d_begin, d_end, dseq_search_window );
                }
                else {
                    /* sequence in channel c ended. change of sequence */

                    M.a[c] = I_MIN;

                    if( d_seq_ptr[c] ) {
                        /* save score */

                        long score = S.a[c] + -I_MIN; // convert score back to range from 0 - 65535

                        if( !overflow.a[c] && (score < UI_MAX) ) {
                            add_to_minheap( heap, q_id, d_seq_ptr[c], score );
                        }
                        else {
                            overflow_chunk->seq[overflow_chunk->fill_pointer++] = d_seq_ptr[c];
                        }

                        done++;
                    }

                    // reset max score
                    S.a[c] = I_MIN;

                    if( next_id < chunk->fill_pointer ) {
                        /* get next sequence with length>0 */

                        d_seq_ptr[c] = chunk->seq[next_id++];

                        d_begin[c] = (unsigned char*) d_seq_ptr[c]->seq.seq;
                        d_end[c] = (unsigned char*) d_seq_ptr[c]->seq.seq + d_seq_ptr[c]->seq.len;

                        no_sequences_ended &= move_db_sequence_window_YY( c, d_begin, d_end, dseq_search_window );
                    }
                    else {
                        /* no more sequences, empty channel */

                        d_seq_ptr[c] = 0;
                        d_begin[c] = 0;
                        d_end[c] = d_begin[c];

                        for( int j = 0; j < CDEPTH; j++ )
                            dseq_search_window[CHANNELS * j + c] = 0;
                    }
                }
            }

            if( done == chunk->fill_pointer )
                break;

            dprofile_fill_YY_xxx( s->dprofile, dseq_search_window );

            aligncolumns_first( &S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, M.v, qlen );
        }

        /*
         * An overflow enforces a sequence change in the corresponding channel,
         * since this sequence has to be re-aligned anyway.
         */
        overflow.v = _mmxxx_cmpeq_epiYY( S.v, score_max );
        no_sequences_ended |= _mmxxx_movemask_epi8( overflow.v );

#ifdef DBG_COLLECT_MATRIX
        d_idx += 4;
#endif
    }

#ifdef DBG_COLLECT_MATRIX
    sequence_t * db_sequences = xmalloc( sizeof( sequence_t ) * done );

    for (int i = 0; i < done; ++i) {
        db_sequences[i] = chunk->seq[i]->seq;
    }

    dbg_print_matrices_to_file( BIT_WIDTH, "SW", s->queries[q_id]->seq, db_sequences, done );

    free( db_sequences );
#endif
}
