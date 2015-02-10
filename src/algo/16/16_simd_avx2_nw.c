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

#include "search_16.h"
#include "search_16_util.h"

#include <limits.h>
#include <string.h>

#include "../../util/util.h"

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

#define ALIGNCORE(H, N, F, V, QR, R, H_MIN, H_MAX)                             \
 H = _mm256_adds_epi16(H, V);            /* add value of scoring matrix */        \
 H = _mm256_max_epi16(H, F);             /* max(H, F) */                          \
 H = _mm256_max_epi16(H, E);             /* max(H, E) */                          \
 H_MIN = _mm256_min_epi16(H_MIN, H);                                              \
 H_MAX = _mm256_max_epi16(H_MAX, H);                                              \
 N = H;                               /* save H in HE-array */                 \
 HF = _mm256_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 F = _mm256_subs_epi16(F, R);            /* subtract gap extend */                \
 F = _mm256_max_epi16(F, HF);            /* test for gap extension, or opening */ \
 HE = _mm256_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 E = _mm256_subs_epi16(E, R);            /* subtract gap extend */                \
 E = _mm256_max_epi16(E, HE);            /* test for gap extension, or opening */

static void aligncolumns_first( __m256i * Sm, __m256i * hep, __m256i ** qp, __m256i gap_open_extend, __m256i gap_extend,
        __m256i h0, __m256i h1, __m256i h2, __m256i h3, __m256i f0, __m256i f1, __m256i f2, __m256i f3,
        __m256i * _h_min, __m256i * _h_max, __m256i Mm, __m256i M_gap_open_extend, __m256i M_gap_extend, long ql ) {
    __m256i h4, h5, h6, h7, h8, E, HE, HF;
    __m256i * vp;
    __m256i h_min = _mm256_setzero_si256();
    __m256i h_max = _mm256_setzero_si256();
    __m256i M_gap_extension = M_gap_open_extend;

    f0 = _mm256_subs_epi16( f0, gap_open_extend );
    f1 = _mm256_subs_epi16( f1, gap_open_extend );
    f2 = _mm256_subs_epi16( f2, gap_open_extend );
    f3 = _mm256_subs_epi16( f3, gap_open_extend );

    for( long i = 0; i < ql; i++ ) {
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
        h4 = _mm256_subs_epu16( h4, Mm );
        h4 = _mm256_subs_epi16( h4, M_gap_extension );

        E = _mm256_subs_epu16( E, Mm );
        E = _mm256_subs_epi16( E, M_gap_extension );
        E = _mm256_subs_epi16( E, M_gap_open_extend );

        M_gap_extension = _mm256_adds_epi16( M_gap_extension, M_gap_extend );

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, h_min, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, h_min, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, h_min, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, h_min, h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_256_16( i, d_idx + 0, h5 );
        dbg_add_matrix_data_256_16( i, d_idx + 1, h6 );
        dbg_add_matrix_data_256_16( i, d_idx + 2, h7 );
        dbg_add_matrix_data_256_16( i, d_idx + 3, h8 );
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

    *_h_min = h_min;
    *_h_max = h_max;
}

static void aligncolumns_rest( __m256i * Sm, __m256i * hep, __m256i ** qp, __m256i gap_open_extend, __m256i gap_extend,
        __m256i h0, __m256i h1, __m256i h2, __m256i h3, __m256i f0, __m256i f1, __m256i f2, __m256i f3,
        __m256i * _h_min, __m256i * _h_max, long ql ) {
    __m256i h4, h5, h6, h7, h8, E, HE, HF;
    __m256i * vp;
    __m256i h_min = _mm256_setzero_si256();
    __m256i h_max = _mm256_setzero_si256();

    f0 = _mm256_subs_epi16( f0, gap_open_extend );
    f1 = _mm256_subs_epi16( f1, gap_open_extend );
    f2 = _mm256_subs_epi16( f2, gap_open_extend );
    f3 = _mm256_subs_epi16( f3, gap_open_extend );

    for( long i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, h_min, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, h_min, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, h_min, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, h_min, h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_256_16( i, d_idx + 0, h5 );
        dbg_add_matrix_data_256_16( i, d_idx + 1, h6 );
        dbg_add_matrix_data_256_16( i, d_idx + 2, h7 );
        dbg_add_matrix_data_256_16( i, d_idx + 3, h8 );
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

    *_h_min = h_min;
    *_h_max = h_max;
}

void search_16_avx2_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int q_id ) {

#ifdef DBG_COLLECT_MATRIX
        dbg_init_matrix_data_collection( BIT_WIDTH_16, s->maxdlen + CDEPTH_16_BIT, s->maxqlen );

        d_idx = 0;
#endif

    int16_t * dprofile = (int16_t*) s->dprofile_avx;
    unsigned long qlen = s->queries[q_id]->q_len;

    __m256i M_gap_open_extend, M_gap_extend;

    __m256i gap_open_extend, gap_extend;

    __m256i * hep;

    uint8_t * d_begin[CHANNELS_16_BIT_AVX];
    uint8_t * d_end[CHANNELS_16_BIT_AVX];
    unsigned long d_length[CHANNELS_16_BIT_AVX];
    p_sdb_sequence d_seq_ptr[CHANNELS_16_BIT_AVX];
    uint8_t overflow[CHANNELS_16_BIT_AVX];

    union {
        __m256i v[CDEPTH_16_BIT];
        int16_t a[CDEPTH_16_BIT * CHANNELS_16_BIT_AVX];
    } S;
    union {
        __m256i v;
        int16_t a[CHANNELS_16_BIT_AVX];
    } M;

    uint8_t dseq_search_window[CDEPTH_16_BIT * CHANNELS_16_BIT_AVX];
    memset( dseq_search_window, 0, CDEPTH_16_BIT * CHANNELS_16_BIT_AVX );

    unsigned long next_id = 0;
    unsigned long done = 0;

    gap_open_extend = _mm256_set1_epi16( s->penalty_gap_open + s->penalty_gap_extension );
    gap_extend = _mm256_set1_epi16( s->penalty_gap_extension );

    hep = s->hearray_avx;

    for( int c = 0; c < CHANNELS_16_BIT_AVX; c++ ) {
        d_begin[c] = 0;
        d_end[c] = d_begin[c];
        d_length[c] = 0;
        d_seq_ptr[c] = 0;
        overflow[c] = 0;
    }

    int16_t score_min = INT16_MIN + s->penalty_gap_open + s->penalty_gap_extension;
    int16_t score_max = INT16_MAX;

    __m256i H0 = _mm256_setzero_si256();
    __m256i H1 = _mm256_setzero_si256();
    __m256i H2 = _mm256_setzero_si256();
    __m256i H3 = _mm256_setzero_si256();

    __m256i F0 = _mm256_setzero_si256();
    __m256i F1 = _mm256_setzero_si256();
    __m256i F2 = _mm256_setzero_si256();
    __m256i F3 = _mm256_setzero_si256();

    union {
        __m256i v;
        int16_t a[CHANNELS_16_BIT_AVX];
    } h_min;
    union {
        __m256i v;
        int16_t a[CHANNELS_16_BIT_AVX];
    } h_max;

    int no_sequences_ended = 0;
    while( 1 ) {
        if( no_sequences_ended ) {
            /* fill all channels with symbols from the database sequences */

            for( int c = 0; c < CHANNELS_16_BIT_AVX; c++ ) {
                no_sequences_ended &= move_db_sequence_window_16( c, CHANNELS_16_BIT_AVX, d_begin, d_end,
                        dseq_search_window );
            }

            dprofile_fill_16_avx2( dprofile, dseq_search_window );

            aligncolumns_rest( S.v, hep, s->queries[q_id]->q_table_avx, gap_open_extend, gap_extend, H0, H1, H2, H3, F0,
                    F1, F2, F3, &h_min.v, &h_max.v, qlen );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            no_sequences_ended = 1;

            M.v = _mm256_setzero_si256();
            for( int c = 0; c < CHANNELS_16_BIT_AVX; c++ ) {
                if( d_begin[c] < d_end[c] ) {
                    /* the sequence in this channel is not finished yet */

                    no_sequences_ended &= move_db_sequence_window_16( c, CHANNELS_16_BIT_AVX, d_begin, d_end,
                            dseq_search_window );
                }
                else {
                    /* sequence in channel c ended. change of sequence */

                    M.a[c] = 0xffff;

                    if( d_seq_ptr[c] ) {
                        /* save score */

                        long z = (d_length[c] + 3) % 4;
                        long score = S.a[z * CHANNELS_16_BIT_AVX + c];

                        if( !overflow[c] && (score > INT16_MIN) && (score < INT16_MAX) ) {
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

                            overflow[c] = 0;
                        }

                        done++;
                    }

                    if( next_id < chunk->fill_pointer ) {
                        /* get next sequence with length>0 */

                        d_seq_ptr[c] = chunk->seq[next_id++];

                        d_length[c] = d_seq_ptr[c]->seq.len;
                        d_begin[c] = (unsigned char*) d_seq_ptr[c]->seq.seq;
                        d_end[c] = (unsigned char*) d_seq_ptr[c]->seq.seq + d_seq_ptr[c]->seq.len;

                        ((int16_t*) &H0)[c] = 0;
                        ((int16_t*) &H1)[c] = -s->penalty_gap_open - 1 * s->penalty_gap_extension;
                        ((int16_t*) &H2)[c] = -s->penalty_gap_open - 2 * s->penalty_gap_extension;
                        ((int16_t*) &H3)[c] = -s->penalty_gap_open - 3 * s->penalty_gap_extension;

                        ((int16_t*) &F0)[c] = -s->penalty_gap_open - 1 * s->penalty_gap_extension;
                        ((int16_t*) &F1)[c] = -s->penalty_gap_open - 2 * s->penalty_gap_extension;
                        ((int16_t*) &F2)[c] = -s->penalty_gap_open - 3 * s->penalty_gap_extension;
                        ((int16_t*) &F3)[c] = -s->penalty_gap_open - 4 * s->penalty_gap_extension;

                        no_sequences_ended &= move_db_sequence_window_16( c, CHANNELS_16_BIT_AVX, d_begin, d_end,
                                dseq_search_window );
                    }
                    else {
                        /* no more sequences, empty channel */

                        d_seq_ptr[c] = 0;
                        d_begin[c] = 0;
                        d_end[c] = d_begin[c];
                        d_length[c] = 0;
                        for( int j = 0; j < CDEPTH_16_BIT; j++ )
                            dseq_search_window[CHANNELS_16_BIT_AVX * j + c] = 0;
                    }
                }
            }

            if( done == chunk->fill_pointer )
                break;

            /* make masked versions of QR, R and E0 */
            M_gap_open_extend = _mm256_and_si256( M.v, gap_open_extend );
            M_gap_extend = _mm256_and_si256( M.v, gap_extend );

            dprofile_fill_16_avx2( dprofile, dseq_search_window );

            aligncolumns_first( S.v, hep, s->queries[q_id]->q_table_avx, gap_open_extend, gap_extend, H0, H1, H2, H3,
                    F0, F1, F2, F3, &h_min.v, &h_max.v, M.v, M_gap_open_extend, M_gap_extend, qlen );
        }
        check_min_max( CHANNELS_16_BIT_AVX, overflow, h_min.a, h_max.a, score_min, score_max );

#ifdef DBG_COLLECT_MATRIX
        d_idx += 4;
#endif

        /*
         * Before calling it again, we need to add CDEPTH * gap_extend to f0, to move it to the new column.
         */
        F0 = _mm256_subs_epi16( F3, gap_extend );
        F1 = _mm256_subs_epi16( F0, gap_extend );
        F2 = _mm256_subs_epi16( F1, gap_extend );
        F3 = _mm256_subs_epi16( F2, gap_extend );

        H0 = _mm256_subs_epi16( H3, gap_extend );
        H1 = _mm256_subs_epi16( H0, gap_extend );
        H2 = _mm256_subs_epi16( H1, gap_extend );
        H3 = _mm256_subs_epi16( H2, gap_extend );
    }

#ifdef DBG_COLLECT_MATRIX
        sequence * db_sequences = xmalloc( sizeof( sequence ) * done );

        for (int i = 0; i < done; ++i) {
            db_sequences[i] = chunk->seq[i]->seq;
        }

        dbg_print_matrices_to_file( BIT_WIDTH_16, "NW", s->queries[q_id]->seq, db_sequences, done );
#endif
}
