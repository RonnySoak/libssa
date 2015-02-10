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

/*
 * is done!!
 *
 * use the range from -32768 - 0 - to 32767
 *
 * init with int8_min and add int8_max to max score
 *
 * add -32768 twice, to reset to zero (modify masking values)
 *
 * a biased Version adds more instructions in ALIGNCORE, than the other version
 */

#ifdef DBG_COLLECT_MATRIX
    static int d_idx;
#endif

#define ALIGNCORE(H, N, F, V, QR, R, S, H_MAX )                         \
 H = _mm_adds_epi8(H, V);            /* add value of scoring matrix */        \
 H = _mm_max_epi8(H, F);             /* max(H, F) */                          \
 H = _mm_max_epi8(H, E);             /* max(H, E) */                          \
 S = _mm_max_epi8(H, S);             /* save max score */                     \
 H_MAX = _mm_max_epi8(H_MAX, H);                                              \
 N = H;                               /* save H in HE-array */                 \
 HF = _mm_subs_epi8(H, QR);          /* subtract gap open-extend */           \
 F = _mm_subs_epi8(F, R);            /* subtract gap extend */                \
 F = _mm_max_epi8(F, HF);            /* test for gap extension, or opening */ \
 HE = _mm_subs_epi8(H, QR);          /* subtract gap open-extend */           \
 E = _mm_subs_epi8(E, R);            /* subtract gap extend */                \
 E = _mm_max_epi8(E, HE);            /* test for gap extension, or opening */

static void aligncolumns_first( __m128i * Sm, __m128i * hep, __m128i ** qp, __m128i gap_open_extend, __m128i gap_extend,
        __m128i Mm, __m128i * _h_max, long ql ) {
    __m128i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m128i * vp;

    __m128i VECTOR_INT8MIN = _mm_set1_epi8( INT8_MIN );

    __m128i h0 = VECTOR_INT8MIN;
    __m128i h1 = VECTOR_INT8MIN;
    __m128i h2 = VECTOR_INT8MIN;
    __m128i h3 = VECTOR_INT8MIN;

    __m128i h_max = VECTOR_INT8MIN;


    f0 = f1 = f2 = f3 = VECTOR_INT8MIN;

    for( long i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        /*
         * h4 and E are set to INT8_MIN for database sequences, that begin here
         *
         * To set both to INT8_MIN, we simply subtract INT8_MIN twice.
         *
         * Mm is set to INT8_MIN on all channels, where new database sequences begin,
         * the other channels are set to zero.
         */

        h4 = _mm_adds_epi8( h4, Mm );
        h4 = _mm_adds_epi8( h4, Mm );

        E = hep[2 * i + 1];
        E = _mm_adds_epi8( E, Mm );
        E = _mm_adds_epi8( E, Mm );

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm, h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_128_8_sw( i, d_idx + 0, h5 );
        dbg_add_matrix_data_128_8_sw( i, d_idx + 1, h6 );
        dbg_add_matrix_data_128_8_sw( i, d_idx + 2, h7 );
        dbg_add_matrix_data_128_8_sw( i, d_idx + 3, h8 );
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

static void aligncolumns_rest( __m128i * Sm, __m128i * hep, __m128i ** qp, __m128i gap_open_extend, __m128i gap_extend,
        __m128i * _h_max, long ql ) {
    __m128i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m128i * vp;

    __m128i VECTOR_INT8MIN = _mm_set1_epi8( INT8_MIN );

    __m128i h0 = VECTOR_INT8MIN;
    __m128i h1 = VECTOR_INT8MIN;
    __m128i h2 = VECTOR_INT8MIN;
    __m128i h3 = VECTOR_INT8MIN;

    __m128i h_max = VECTOR_INT8MIN;
    
    f0 = f1 = f2 = f3 = VECTOR_INT8MIN;

    for( long i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm, h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_128_8_sw( i, d_idx + 0, h5 );
        dbg_add_matrix_data_128_8_sw( i, d_idx + 1, h6 );
        dbg_add_matrix_data_128_8_sw( i, d_idx + 2, h7 );
        dbg_add_matrix_data_128_8_sw( i, d_idx + 3, h8 );
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

void search_8_sse41_sw( p_s8info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int q_id ) {

#ifdef DBG_COLLECT_MATRIX
        dbg_init_matrix_data_collection( BIT_WIDTH_8, s->maxdlen + CDEPTH_8_BIT, s->maxqlen );

        d_idx = 0;
#endif

    int8_t * dprofile = (int8_t*) s->dprofile_sse;
    unsigned long qlen = s->queries[q_id]->q_len;

    __m128i T, M, T0;

    __m128i gap_open_extend, gap_extend;

    __m128i * hep;

    uint8_t * d_begin[CHANNELS_8_BIT_SSE];
    uint8_t * d_end[CHANNELS_8_BIT_SSE];
    p_sdb_sequence d_seq_ptr[CHANNELS_8_BIT_SSE];
    uint8_t overflow[CHANNELS_8_BIT_SSE];

    union {
        __m128i v;
        int8_t a[CHANNELS_8_BIT_SSE];
    } S;

    uint8_t dseq_search_window[CDEPTH_8_BIT * CHANNELS_8_BIT_SSE];
    memset( dseq_search_window, 0, CDEPTH_8_BIT * CHANNELS_8_BIT_SSE );

    unsigned long next_id = 0;
    unsigned long done = 0;

    T0 = _mm_set_epi8( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, INT8_MIN );

    gap_open_extend = _mm_set1_epi8( s->penalty_gap_open + s->penalty_gap_extension );
    gap_extend = _mm_set1_epi8( s->penalty_gap_extension );

    hep = s->hearray_sse;

    for( int c = 0; c < CHANNELS_8_BIT_SSE; c++ ) {
        d_begin[c] = 0;
        d_end[c] = d_begin[c];
        d_seq_ptr[c] = 0;
        overflow[c] = 0;
    }

    int8_t score_max = INT8_MAX;

    union {
        __m128i v;
        int8_t a[CHANNELS_8_BIT_SSE];
    } h_max;
    
    int no_sequences_ended = 0;
    while( 1 ) {
        if( no_sequences_ended ) {
            /* fill all channels with symbols from the database sequences */

            for( int c = 0; c < CHANNELS_8_BIT_SSE; c++ ) {
                no_sequences_ended &= move_db_sequence_window_8( c, CHANNELS_8_BIT_SSE, d_begin, d_end, dseq_search_window );
            }

            dprofile_fill_8_sse41( dprofile, dseq_search_window );

            aligncolumns_rest( &S.v, hep, s->queries[q_id]->q_table_sse, gap_open_extend, gap_extend, &h_max.v, qlen );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            no_sequences_ended = 1;

            M = _mm_setzero_si128();
            T = T0;
            for( int c = 0; c < CHANNELS_8_BIT_SSE; c++ ) {
                if( d_begin[c] < d_end[c] ) {
                    /* the sequence in this channel is not finished yet */

                    no_sequences_ended &= move_db_sequence_window_8( c, CHANNELS_8_BIT_SSE, d_begin, d_end, dseq_search_window );
                }
                else {
                    /* sequence in channel c ended. change of sequence */

                    M = _mm_xor_si128( M, T );

                    if( d_seq_ptr[c] ) {
                        /* save score */

                        long score = S.a[c] + -INT8_MIN; // convert score back to range from 0 - 65535

                        if( !overflow[c] && (score >= 0) && (score < UINT8_MAX) ) {
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

                    // reset max score
                    S.a[c] = INT8_MIN;

                    if( next_id < chunk->fill_pointer ) {
                        /* get next sequence with length>0 */

                        d_seq_ptr[c] = chunk->seq[next_id++];

                        d_begin[c] = (unsigned char*) d_seq_ptr[c]->seq.seq;
                        d_end[c] = (unsigned char*) d_seq_ptr[c]->seq.seq + d_seq_ptr[c]->seq.len;

                        no_sequences_ended &= move_db_sequence_window_8( c, CHANNELS_8_BIT_SSE, d_begin, d_end, dseq_search_window );
                    }
                    else {
                        /* no more sequences, empty channel */

                        d_seq_ptr[c] = 0;
                        d_begin[c] = 0;
                        d_end[c] = d_begin[c];

                        for( int j = 0; j < CDEPTH_8_BIT; j++ )
                            dseq_search_window[CHANNELS_8_BIT_SSE * j + c] = 0;
                    }
                }

                T = _mm_slli_si128( T, 1 );
            }

            if( done == chunk->fill_pointer )
                break;

            dprofile_fill_8_sse41( dprofile, dseq_search_window );

            aligncolumns_first( &S.v, hep, s->queries[q_id]->q_table_sse, gap_open_extend, gap_extend, M, &h_max.v, qlen );
        }
        check_max( CHANNELS_8_BIT_SSE, overflow, h_max.a, score_max );

#ifdef DBG_COLLECT_MATRIX
        d_idx += 4;
#endif
    }

#ifdef DBG_COLLECT_MATRIX
        sequence * db_sequences = xmalloc( sizeof( sequence ) * done );

        for (int i = 0; i < done; ++i) {
            db_sequences[i] = chunk->seq[i]->seq;
        }

        dbg_print_matrices_to_file( BIT_WIDTH_8, "SW", s->queries[q_id]->seq, db_sequences, done );
#endif
}
