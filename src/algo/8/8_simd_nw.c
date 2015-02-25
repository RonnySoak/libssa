/*
 Copyright (C) 2012-2014 Torbjorn Rognes & Frederic Mahe

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

#include "search_8.h"
#include "search_8_util.h"

#include <limits.h>
#include <string.h>

#include "../../util/util.h"
#ifdef __AVX2__

typedef __m256i __mxxxi;
#define _mmxxx_and_si _mm256_and_si256
#define _mmxxx_adds_epi8 _mm256_adds_epi8
#define _mmxxx_subs_epi8 _mm256_subs_epi8
#define _mmxxx_subs_epu8 _mm256_subs_epu8
#define _mmxxx_max_epi8 _mm256_max_epi8
#define _mmxxx_min_epi8 _mm256_min_epi8
#define _mmxxx_set1_epi8 _mm256_set1_epi8
#define _mmxxx_setzero_si _mm256_setzero_si256
#define _mmxxx_cmpgt_epi8 _mm256_cmpgt_epi8
#define _mmxxx_or_si _mm256_or_si256
#define _mmxxx_cmpeq_epi8 _mm256_cmpeq_epi8
#define dprofile_fill_8_xxx dprofile_fill_8_avx2
#define dbg_add_matrix_data_xxx_8 dbg_add_matrix_data_256_8

#else // SSE2

typedef __m128i  __mxxxi;
#define _mmxxx_and_si _mm_and_si128
#define _mmxxx_adds_epi8 _mm_adds_epi8
#define _mmxxx_subs_epi8 _mm_subs_epi8
#define _mmxxx_subs_epu8 _mm_subs_epu8
#define _mmxxx_max_epi8 _mm_max_epi8
#define _mmxxx_min_epi8 _mm_min_epi8
#define _mmxxx_set1_epi8 _mm_set1_epi8
#define _mmxxx_setzero_si _mm_setzero_si128
#define _mmxxx_cmpgt_epi8 _mm_cmpgt_epi8
#define _mmxxx_or_si _mm_or_si128
#define _mmxxx_cmpeq_epi8 _mm_cmpeq_epi8
#define dprofile_fill_8_xxx dprofile_fill_8_sse41
#define dbg_add_matrix_data_xxx_8 dbg_add_matrix_data_128_8

#endif
/*
 Using 8-bit signed values, from -32768 to +32767.
 match: positive
 mismatch: negative
 gap penalties: positive (open, extend)
 optimal global alignment (NW)
 maximize score
 */
#ifdef DBG_COLLECT_MATRIX
static int d_idx;
#endif

#define ALIGNCORE(H, N, F, V, QR, R, H_MIN, H_MAX)                             \
 H = _mmxxx_adds_epi8(H, V);          /* add value of scoring matrix */        \
 H = _mmxxx_max_epi8(H, F);           /* max(H, F) */                          \
 H = _mmxxx_max_epi8(H, E);           /* max(H, E) */                          \
 H_MIN = _mmxxx_min_epi8(H_MIN, H);                                            \
 H_MAX = _mmxxx_max_epi8(H_MAX, H);                                            \
 N = H;                               /* save H in HE-array */                 \
 HF = _mmxxx_subs_epi8(H, QR);        /* subtract gap open-extend */           \
 F = _mmxxx_subs_epi8(F, R);          /* subtract gap extend */                \
 F = _mmxxx_max_epi8(F, HF);          /* test for gap extension, or opening */ \
 HE = _mmxxx_subs_epi8(H, QR);        /* subtract gap open-extend */           \
 E = _mmxxx_subs_epi8(E, R);          /* subtract gap extend */                \
 E = _mmxxx_max_epi8(E, HE);          /* test for gap extension, or opening */

static void aligncolumns_first( __mxxxi * Sm, __mxxxi * hep, __mxxxi ** qp, __mxxxi gap_open_extend, __mxxxi gap_extend,
        __mxxxi h0, __mxxxi h1, __mxxxi h2, __mxxxi h3, __mxxxi f0, __mxxxi f1, __mxxxi f2, __mxxxi f3,
        __mxxxi * _h_min, __mxxxi * _h_max, __mxxxi Mm, __mxxxi M_gap_open_extend, __mxxxi M_gap_extend, size_t ql ) {
    __mxxxi h4, h5, h6, h7, h8, E, HE, HF;
    __mxxxi * vp;
    /*
     * We set h_min and h_max to zero, to prevent a reuse of the previous score
     * in the same channel.
     */
    *_h_min = _mmxxx_setzero_si();
    *_h_max = _mmxxx_setzero_si();

    __mxxxi M_gap_extension = M_gap_open_extend;

    f0 = _mmxxx_subs_epi8( f0, gap_open_extend );
    f1 = _mmxxx_subs_epi8( f1, gap_open_extend );
    f2 = _mmxxx_subs_epi8( f2, gap_open_extend );
    f3 = _mmxxx_subs_epi8( f3, gap_open_extend );

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
        h4 = _mmxxx_subs_epu8( h4, Mm );
        h4 = _mmxxx_subs_epi8( h4, M_gap_extension );

        E = _mmxxx_subs_epu8( E, Mm );
        E = _mmxxx_subs_epi8( E, M_gap_extension );
        E = _mmxxx_subs_epi8( E, M_gap_open_extend );

        M_gap_extension = _mmxxx_adds_epi8( M_gap_extension, M_gap_extend );

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *_h_min, *_h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_xxx_8( i, d_idx + 0, h5 );
        dbg_add_matrix_data_xxx_8( i, d_idx + 1, h6 );
        dbg_add_matrix_data_xxx_8( i, d_idx + 2, h7 );
        dbg_add_matrix_data_xxx_8( i, d_idx + 3, h8 );
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
    __mxxxi h4, h5, h6, h7, h8, E, HE, HF;
    __mxxxi * vp;

    f0 = _mmxxx_subs_epi8( f0, gap_open_extend );
    f1 = _mmxxx_subs_epi8( f1, gap_open_extend );
    f2 = _mmxxx_subs_epi8( f2, gap_open_extend );
    f3 = _mmxxx_subs_epi8( f3, gap_open_extend );

    for( size_t i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *_h_min, *_h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *_h_min, *_h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_xxx_8( i, d_idx + 0, h5 );
        dbg_add_matrix_data_xxx_8( i, d_idx + 1, h6 );
        dbg_add_matrix_data_xxx_8( i, d_idx + 2, h7 );
        dbg_add_matrix_data_xxx_8( i, d_idx + 3, h8 );
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


#ifdef __AVX2__
void search_8_avx2_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int q_id ) {
#else
void search_8_sse41_nw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int q_id ) {
#endif

#ifdef DBG_COLLECT_MATRIX
    size_t maxdlen =  0;
    for( int i = 0; i < chunk->fill_pointer; ++i ) {
        if( maxdlen < chunk->seq[i]->seq.len ) {
            maxdlen = chunk->seq[i]->seq.len;
        }
    }

    dbg_init_matrix_data_collection( BIT_WIDTH_8, maxdlen + CDEPTH_8_BIT, s->maxqlen );

    d_idx = 0;
#endif

    size_t qlen = s->queries[q_id]->q_len;

    __mxxxi M_gap_open_extend, M_gap_extend;

    __mxxxi gap_open_extend, gap_extend;

    __mxxxi * hep;

    uint8_t * d_begin[CHANNELS_8_BIT];
    uint8_t * d_end[CHANNELS_8_BIT];
    size_t d_length[CHANNELS_8_BIT];
    p_sdb_sequence d_seq_ptr[CHANNELS_8_BIT];

    union {
        __mxxxi v[CDEPTH_8_BIT];
        int8_t a[CDEPTH_8_BIT * CHANNELS_8_BIT];
    } S;
    union {
        __mxxxi v;
        int8_t a[CHANNELS_8_BIT];
    } M;
    union {
        __mxxxi v;
        int8_t a[CHANNELS_8_BIT];
    } overflow;

    overflow.v = _mmxxx_setzero_si();

    uint16_t dseq_search_window[CDEPTH_8_BIT * CHANNELS_8_BIT];

    size_t next_id = 0;
    size_t done = 0;

    gap_open_extend = _mmxxx_set1_epi8( s->penalty_gap_open + s->penalty_gap_extension );
    gap_extend = _mmxxx_set1_epi8( s->penalty_gap_extension );

    hep = s->hearray;

    for( int c = 0; c < CHANNELS_8_BIT; c++ ) {
        d_begin[c] = 0;
        d_end[c] = d_begin[c];
        d_length[c] = 0;
        d_seq_ptr[c] = 0;
    }

    __mxxxi score_min = _mmxxx_set1_epi8( INT8_MIN + s->penalty_gap_open + s->penalty_gap_extension -1 );
    __mxxxi score_max = _mmxxx_set1_epi8( INT8_MAX );

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

    int no_sequences_ended = 0;
    while( 1 ) {
        if( no_sequences_ended ) {
            /* fill all channels with symbols from the database sequences */

            for( int c = 0; c < CHANNELS_8_BIT; c++ ) {
                if( d_seq_ptr[c] )
                    no_sequences_ended &= move_db_sequence_window_8( c, d_begin, d_end, dseq_search_window );
            }

            dprofile_fill_8_xxx( s->dprofile, dseq_search_window );

            aligncolumns_rest( S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, H0, H1, H2, H3, F0, F1,
                    F2, F3, &h_min, &h_max, qlen );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            no_sequences_ended = 1;

            M.v = _mmxxx_setzero_si();
            for( int c = 0; c < CHANNELS_8_BIT; c++ ) {
                if( d_begin[c] < d_end[c] ) {
                    /* the sequence in this channel is not finished yet */

                    no_sequences_ended &= move_db_sequence_window_8( c, d_begin, d_end, dseq_search_window );
                }
                else {
                    /* sequence in channel c ended. change of sequence */

                    M.a[c] = 0xff;

                    if( d_seq_ptr[c] ) {
                        /* save score */

                        long z = (d_length[c] + 3) % 4;
                        long score = S.a[z * CHANNELS_8_BIT + c];

                        if( !overflow.a[c] && (score > INT8_MIN) && (score < INT8_MAX) ) {
                            /* Alignments, with a score equal to the current lowest score in the
                             heap are ignored! */
                            add_to_minheap( heap, q_id, d_seq_ptr[c], score );
                        }
                        else {
                            if( *overflow_list ) {
                                ll_push( overflow_list, d_seq_ptr[c] );
                            }
                            else {
                                *overflow_list = ll_init( d_seq_ptr[c] );
                            }

                            overflow.a[c] = 0;
                        }

                        done++;
                    }

                    if( next_id < chunk->fill_pointer ) {
                        /* get next sequence with length>0 */

                        d_seq_ptr[c] = chunk->seq[next_id++];

                        d_length[c] = d_seq_ptr[c]->seq.len;
                        d_begin[c] = (unsigned char*) d_seq_ptr[c]->seq.seq;
                        d_end[c] = (unsigned char*) d_seq_ptr[c]->seq.seq + d_seq_ptr[c]->seq.len;

                        /*
                         * TODO: Could be changed into a masked operation after this loop.
                         * The mask is created anyway and if we have to do this multiple times,
                         * we can do it masked as well.
                         *
                         * Maybe combine it with the instructions at the end of the outer loop,
                         * where the H and F values are changed, to the next column.
                         *
                         * By combining it, we just have to reset F3 and H3 and then apply the
                         * standard changes: H3 = (masked) 0; F3 = (masked) penalty_gap_open
                         */
                        ((int8_t*) &H0)[c] = 0;
                        ((int8_t*) &H1)[c] = -s->penalty_gap_open - 1 * s->penalty_gap_extension;
                        ((int8_t*) &H2)[c] = -s->penalty_gap_open - 2 * s->penalty_gap_extension;
                        ((int8_t*) &H3)[c] = -s->penalty_gap_open - 3 * s->penalty_gap_extension;

                        ((int8_t*) &F0)[c] = -s->penalty_gap_open - 1 * s->penalty_gap_extension;
                        ((int8_t*) &F1)[c] = -s->penalty_gap_open - 2 * s->penalty_gap_extension;
                        ((int8_t*) &F2)[c] = -s->penalty_gap_open - 3 * s->penalty_gap_extension;
                        ((int8_t*) &F3)[c] = -s->penalty_gap_open - 4 * s->penalty_gap_extension;

                        no_sequences_ended &= move_db_sequence_window_8( c, d_begin, d_end, dseq_search_window );
                    }
                    else {
                        /* no more sequences, empty channel */

                        /*
                         * TODO "switch off" empty channel, when the chunk is empty.
                         *
                         * This could prevent a constant switching between align_cloumns_first
                         * and align_columns_rest ...
                         */

                        d_seq_ptr[c] = 0;
                        d_begin[c] = 0;
                        d_end[c] = d_begin[c];
                        d_length[c] = 0;
                        for( int j = 0; j < CDEPTH_8_BIT; j++ )
                            dseq_search_window[CHANNELS_8_BIT * j + c] = 0;
                    }
                }
            }

            if( done == chunk->fill_pointer )
                break;

            /* make masked versions of QR, R and E0 */
            M_gap_open_extend = _mmxxx_and_si( M.v, gap_open_extend );
            M_gap_extend = _mmxxx_and_si( M.v, gap_extend );

            dprofile_fill_8_xxx( s->dprofile, dseq_search_window );

            aligncolumns_first( S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, H0, H1, H2, H3,
                    F0, F1, F2, F3, &h_min, &h_max, M.v, M_gap_open_extend, M_gap_extend, qlen );
        }

        overflow.v = _mmxxx_or_si( _mmxxx_cmpgt_epi8( score_min, h_min ), overflow.v );
        overflow.v = _mmxxx_or_si( _mmxxx_cmpeq_epi8( h_max, score_max ), overflow.v );

#ifdef DBG_COLLECT_MATRIX
        d_idx += 4;
#endif

        /*
         * Before calling it again, we need to add CDEPTH * gap_extend to f0, to move it to the new column.
         */
        F0 = _mmxxx_subs_epi8( F3, gap_extend );
        F1 = _mmxxx_subs_epi8( F0, gap_extend );
        F2 = _mmxxx_subs_epi8( F1, gap_extend );
        F3 = _mmxxx_subs_epi8( F2, gap_extend );

        H0 = _mmxxx_subs_epi8( H3, gap_extend );
        H1 = _mmxxx_subs_epi8( H0, gap_extend );
        H2 = _mmxxx_subs_epi8( H1, gap_extend );
        H3 = _mmxxx_subs_epi8( H2, gap_extend );
    }

#ifdef DBG_COLLECT_MATRIX
    sequence * db_sequences = xmalloc( sizeof( sequence ) * done );

    for (int i = 0; i < done; ++i) {
        db_sequences[i] = chunk->seq[i]->seq;
    }

    dbg_print_matrices_to_file( BIT_WIDTH_8, "NW", s->queries[q_id]->seq, db_sequences, done );

    free( db_sequences );
#endif
}
