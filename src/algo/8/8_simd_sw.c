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

#include "search_8.h"
#include "search_8_util.h"

#include <limits.h>
#include <string.h>

#include "../../util/util.h"

#ifdef __AVX2__

#define _mmxxx_adds_epi8 _mm256_adds_epi8
#define _mmxxx_subs_epi8 _mm256_subs_epi8
#define _mmxxx_max_epi8 _mm256_max_epi8
#define _mmxxx_set1_epi8 _mm256_set1_epi8
#define _mmxxx_setzero_si _mm256_setzero_si256
#define _mmxxx_or_si _mm256_or_si256
#define _mmxxx_cmpeq_epi8 _mm256_cmpeq_epi8
#define dprofile_fill_8_xxx dprofile_fill_8_avx2
#define dbg_add_matrix_data_xxx_8_sw dbg_add_matrix_data_256_8_sw

#else // SSE4.1

#define _mmxxx_adds_epi8 _mm_adds_epi8
#define _mmxxx_subs_epi8 _mm_subs_epi8
#define _mmxxx_max_epi8 _mm_max_epi8
#define _mmxxx_set1_epi8 _mm_set1_epi8
#define _mmxxx_setzero_si _mm_setzero_si128
#define _mmxxx_or_si _mm_or_si128
#define _mmxxx_cmpeq_epi8 _mm_cmpeq_epi8
#define dprofile_fill_8_xxx dprofile_fill_8_sse41
#define dbg_add_matrix_data_xxx_8_sw dbg_add_matrix_data_128_8_sw

#endif
/*
 * use the range from -128 - 0 - to +127
 *
 * init with int8_min and add int8_max to max score
 *
 * add -128 twice, to reset to zero (modify masking values)
 *
 * a biased Version adds more instructions in ALIGNCORE, than the other version
 */
#ifdef DBG_COLLECT_MATRIX
static int d_idx;
#endif

#define ALIGNCORE(H, N, F, V, QR, R, S, H_MAX )                                  \
 H = _mmxxx_adds_epi8(H, V);            /* add value of scoring profile */        \
 H = _mmxxx_max_epi8(H, F);             /* max(H, F) */                          \
 H = _mmxxx_max_epi8(H, E);             /* max(H, E) */                          \
 S = _mmxxx_max_epi8(H, S);             /* save max score */                     \
 H_MAX = _mmxxx_max_epi8(H_MAX, H);                                              \
 N = H;                                 /* save H in HE-array */                 \
 HQR = _mmxxx_subs_epi8(H, QR);         /* subtract gap open-extend */           \
 F = _mmxxx_subs_epi8(F, R);            /* subtract gap extend */                \
 F = _mmxxx_max_epi8(F, HQR);           /* test for gap extension, or opening */ \
 E = _mmxxx_subs_epi8(E, R);            /* subtract gap extend */                \
 E = _mmxxx_max_epi8(E, HQR);           /* test for gap extension, or opening */

static void aligncolumns_first( __mxxxi * Sm, __mxxxi * hep, __mxxxi ** qp, __mxxxi gap_open_extend, __mxxxi gap_extend,
        __mxxxi Mm, __mxxxi * _h_max, size_t ql ) {
    __mxxxi h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HQR;
    __mxxxi * vp;

    __mxxxi VECTOR_INT8MIN = _mmxxx_set1_epi8( INT8_MIN );

    __mxxxi h0 = VECTOR_INT8MIN;
    __mxxxi h1 = VECTOR_INT8MIN;
    __mxxxi h2 = VECTOR_INT8MIN;
    __mxxxi h3 = VECTOR_INT8MIN;

    __mxxxi h_max = VECTOR_INT8MIN;

    f0 = f1 = f2 = f3 = VECTOR_INT8MIN;

    for( size_t i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        /*
         * h4 and E are set to INT8_MIN for database sequences, that begin here
         *
         * To set both to INT8_MIN, we simply subtract INT8_MIN twice.
         *
         * Mm is set to INT8_MIN on all channels, where new database sequences begin,
         * the other channels are not affected.
         */

        h4 = _mmxxx_adds_epi8( h4, Mm );
        h4 = _mmxxx_adds_epi8( h4, Mm );

        E = hep[2 * i + 1];
        E = _mmxxx_adds_epi8( E, Mm );
        E = _mmxxx_adds_epi8( E, Mm );

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm, h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_xxx_8_sw( i, d_idx + 0, h5 );
        dbg_add_matrix_data_xxx_8_sw( i, d_idx + 1, h6 );
        dbg_add_matrix_data_xxx_8_sw( i, d_idx + 2, h7 );
        dbg_add_matrix_data_xxx_8_sw( i, d_idx + 3, h8 );
#endif

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;
    }

    *_h_max = h_max;
}

static void aligncolumns_rest( __mxxxi * Sm, __mxxxi * hep, __mxxxi ** qp, __mxxxi gap_open_extend, __mxxxi gap_extend,
        __mxxxi * _h_max, size_t ql ) {
    __mxxxi h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HQR;
    __mxxxi * vp;

    __mxxxi VECTOR_INT8MIN = _mmxxx_set1_epi8( INT8_MIN );

    __mxxxi h0 = VECTOR_INT8MIN;
    __mxxxi h1 = VECTOR_INT8MIN;
    __mxxxi h2 = VECTOR_INT8MIN;
    __mxxxi h3 = VECTOR_INT8MIN;

    f0 = f1 = f2 = f3 = VECTOR_INT8MIN;

    for( size_t i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm, *_h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm, *_h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm, *_h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm, *_h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_xxx_8_sw( i, d_idx + 0, h5 );
        dbg_add_matrix_data_xxx_8_sw( i, d_idx + 1, h6 );
        dbg_add_matrix_data_xxx_8_sw( i, d_idx + 2, h7 );
        dbg_add_matrix_data_xxx_8_sw( i, d_idx + 3, h8 );
#endif

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;
    }
}

#ifdef __AVX2__
void search_8_avx2_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, uint8_t q_id ) {
#else
void search_8_sse41_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, uint8_t q_id ) {
#endif

#ifdef DBG_COLLECT_MATRIX
    size_t maxdlen = 0;
    for( int i = 0; i < chunk->fill_pointer; ++i ) {
        if( maxdlen < chunk->seq[i]->seq.len ) {
            maxdlen = chunk->seq[i]->seq.len;
        }
    }

    dbg_init_matrix_data_collection( BIT_WIDTH_8, maxdlen + CDEPTH_8_BIT, s->maxqlen );

    d_idx = 0;
#endif

    size_t qlen = s->queries[q_id]->q_len;

    __mxxxi gap_open_extend, gap_extend;

    __mxxxi * hep;

    uint8_t * d_begin[CHANNELS_8_BIT];
    uint8_t * d_end[CHANNELS_8_BIT];
    p_sdb_sequence d_seq_ptr[CHANNELS_8_BIT];

    union {
        __mxxxi v;
        int8_t a[CHANNELS_8_BIT];
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
        d_seq_ptr[c] = 0;
    }

    __mxxxi score_max = _mmxxx_set1_epi8( INT8_MAX );

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

            aligncolumns_rest( &S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, &h_max, qlen );
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

                    M.a[c] = INT8_MIN;

                    if( d_seq_ptr[c] ) {
                        /* save score */

                        long score = S.a[c] + -INT8_MIN; // convert back to a range of 0 to 255

                        if( !overflow.a[c] && (score > 0) && (score < UINT8_MAX) ) {
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

                    // reset max score
                    S.a[c] = INT8_MIN;

                    if( next_id < chunk->fill_pointer ) {
                        /* get next sequence with length>0 */

                        d_seq_ptr[c] = chunk->seq[next_id++];

                        d_begin[c] = (unsigned char*) d_seq_ptr[c]->seq.seq;
                        d_end[c] = (unsigned char*) d_seq_ptr[c]->seq.seq + d_seq_ptr[c]->seq.len;

                        no_sequences_ended &= move_db_sequence_window_8( c, d_begin, d_end, dseq_search_window );
                    }
                    else {
                        /* no more sequences, empty channel */

                        d_seq_ptr[c] = 0;
                        d_begin[c] = 0;
                        d_end[c] = d_begin[c];

                        for( int j = 0; j < CDEPTH_8_BIT; j++ )
                            dseq_search_window[CHANNELS_8_BIT * j + c] = 0;
                    }
                }
            }

            if( done == chunk->fill_pointer )
                break;

            dprofile_fill_8_xxx( s->dprofile, dseq_search_window );

            aligncolumns_first( &S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, M.v, &h_max, qlen );
        }
        overflow.v = _mmxxx_or_si( _mmxxx_cmpeq_epi8( h_max, score_max ), overflow.v );

#ifdef DBG_COLLECT_MATRIX
        d_idx += 4;
#endif
    }

#ifdef DBG_COLLECT_MATRIX
    sequence_t * db_sequences = xmalloc( sizeof( sequence_t ) * done );

    for (int i = 0; i < done; ++i) {
        db_sequences[i] = chunk->seq[i]->seq;
    }

    dbg_print_matrices_to_file( BIT_WIDTH_8, "SW", s->queries[q_id]->seq, db_sequences, done );

    free( db_sequences );
#endif
}
