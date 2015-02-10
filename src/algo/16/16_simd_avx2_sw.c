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
 * use the range from -32768 - 0 - to 32767
 *
 * init with int16_min and add int16_max to max score
 *
 * add -32768 twice, to reset to zero (modify masking values)
 *
 * a biased Version adds more instructions in ALIGNCORE, than the other version
 */

#ifdef DBG_COLLECT_MATRIX
    static int d_idx;
#endif

#define ALIGNCORE(H, N, F, V, QR, R, S, H_MAX )                         	      \
 H = _mm256_adds_epi16(H, V);            /* add value of scoring matrix */        \
 H = _mm256_max_epi16(H, F);             /* max(H, F) */                          \
 H = _mm256_max_epi16(H, E);             /* max(H, E) */                          \
 S = _mm256_max_epi16(H, S);             /* save max score */                     \
 H_MAX = _mm256_max_epi16(H_MAX, H);                                              \
 N = H;                                  /* save H in HE-array */                 \
 HF = _mm256_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 F = _mm256_subs_epi16(F, R);            /* subtract gap extend */                \
 F = _mm256_max_epi16(F, HF);            /* test for gap extension, or opening */ \
 HE = _mm256_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 E = _mm256_subs_epi16(E, R);            /* subtract gap extend */                \
 E = _mm256_max_epi16(E, HE);            /* test for gap extension, or opening */

static void aligncolumns_first( __m256i * Sm, __m256i * hep, __m256i ** qp, __m256i gap_open_extend, __m256i gap_extend,
        __m256i Mm, __m256i * _h_max, long ql ) {
    __m256i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m256i * vp;

    __m256i VECTOR_INT16MIN = _mm256_set1_epi16( INT16_MIN );

    __m256i h0 = VECTOR_INT16MIN;
    __m256i h1 = VECTOR_INT16MIN;
    __m256i h2 = VECTOR_INT16MIN; // TODO difference to 8 bit SW search!!!!
    __m256i h3 = VECTOR_INT16MIN;

    __m256i h_max = VECTOR_INT16MIN;

    f0 = f1 = f2 = f3 = VECTOR_INT16MIN;

    for( long i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        /*
         * h4 and E are set to INT16_MIN for database sequences, that begin here
         *
         * To set both to INT16_MIN, we simply subtract INT16_MIN twice.
         *
         * Mm is set to INT16_MIN on all channels, where new database sequences begin,
         * the other channels are set to zero.
         */

        h4 = _mm256_subs_epi16( h4, Mm );
        h4 = _mm256_subs_epi16( h4, Mm );

        E = hep[2 * i + 1];
        E = _mm256_subs_epi16( E, Mm );
        E = _mm256_subs_epi16( E, Mm );

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm, h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_256_16_sw( i, d_idx + 0, h5 );
        dbg_add_matrix_data_256_16_sw( i, d_idx + 1, h6 );
        dbg_add_matrix_data_256_16_sw( i, d_idx + 2, h7 );
        dbg_add_matrix_data_256_16_sw( i, d_idx + 3, h8 );
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

static void aligncolumns_rest( __m256i * Sm, __m256i * hep, __m256i ** qp, __m256i gap_open_extend, __m256i gap_extend,
        __m256i * _h_max, long ql ) {
    __m256i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m256i * vp;

    __m256i VECTOR_INT16MIN = _mm256_set1_epi16( INT16_MIN );

    __m256i h0 = VECTOR_INT16MIN;
    __m256i h1 = VECTOR_INT16MIN;
    __m256i h2 = VECTOR_INT16MIN;
    __m256i h3 = VECTOR_INT16MIN;

    __m256i h_max = VECTOR_INT16MIN;

    f0 = f1 = f2 = f3 = VECTOR_INT16MIN;

    for( long i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm, h_max );

#ifdef DBG_COLLECT_MATRIX
        dbg_add_matrix_data_256_16_sw( i, d_idx + 0, h5 );
        dbg_add_matrix_data_256_16_sw( i, d_idx + 1, h6 );
        dbg_add_matrix_data_256_16_sw( i, d_idx + 2, h7 );
        dbg_add_matrix_data_256_16_sw( i, d_idx + 3, h8 );
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

void search_16_avx2_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, p_node * overflow_list, int q_id ) {

#ifdef DBG_COLLECT_MATRIX
        dbg_init_matrix_data_collection( BIT_WIDTH_16, s->maxdlen + CDEPTH_16_BIT, s->maxqlen );

        d_idx = 0;
#endif

    int16_t * dprofile = (int16_t*) s->dprofile_avx;
    unsigned long qlen = s->queries[q_id]->q_len;

    __m256i gap_open_extend, gap_extend;

    __m256i * hep;

    uint8_t * d_begin[CHANNELS_16_BIT_AVX];
    uint8_t * d_end[CHANNELS_16_BIT_AVX];
    p_sdb_sequence d_seq_ptr[CHANNELS_16_BIT_AVX];
    uint8_t overflow[CHANNELS_16_BIT_AVX];

    union {
        __m256i v;
        int16_t a[CHANNELS_16_BIT_AVX];
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
        d_seq_ptr[c] = 0;
        overflow[c] = 0;
    }

    int16_t score_max = INT16_MAX;

    union {
        __m256i v;
        int16_t a[CHANNELS_16_BIT_AVX];
    } h_max;

    int no_sequences_ended = 0;
    while( 1 ) {
        /*
         * TODO If there are less sequences, as channels, this loop switches constantly between
         * the states "sequences ended" and "no sequences ended". Check if it is possible, to "disable"
         * the "sequence ended" checks, for these unused channels.
         *
         * TODO same for NW implementation! Check if it is possible, to merge NW and SW implementations.
         */

        if( no_sequences_ended ) {
            /* fill all channels with symbols from the database sequences */

            for( int c = 0; c < CHANNELS_16_BIT_AVX; c++ ) {
                no_sequences_ended &= move_db_sequence_window_16( c, CHANNELS_16_BIT_AVX, d_begin, d_end, dseq_search_window );
            }

            dprofile_fill_16_avx2( dprofile, dseq_search_window );

            aligncolumns_rest( &S.v, hep, s->queries[q_id]->q_table_avx, gap_open_extend, gap_extend, &h_max.v, qlen );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            no_sequences_ended = 1;

            M.v = _mm256_setzero_si256();
            for( int c = 0; c < CHANNELS_16_BIT_AVX; c++ ) {
                if( d_begin[c] < d_end[c] ) {
                    /* the sequence in this channel is not finished yet */

                    no_sequences_ended &= move_db_sequence_window_16( c, CHANNELS_16_BIT_AVX, d_begin, d_end, dseq_search_window );
                }
                else {
                    /* sequence in channel c ended. change of sequence */

                    M.a[c] = INT16_MAX;

                    if( d_seq_ptr[c] ) {
                        /* save score */

                        long score = S.a[c] + -INT16_MIN; // convert score back to range from 0 - 65535

                        if( !overflow[c] && (score >= 0) && (score < UINT16_MAX) ) {
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
                    S.a[c] = INT16_MIN;

                    if( next_id < chunk->fill_pointer ) {
                        /* get next sequence with length>0 */

                        d_seq_ptr[c] = chunk->seq[next_id++];

                        d_begin[c] = (unsigned char*) d_seq_ptr[c]->seq.seq;
                        d_end[c] = (unsigned char*) d_seq_ptr[c]->seq.seq + d_seq_ptr[c]->seq.len;

                        no_sequences_ended &= move_db_sequence_window_16( c, CHANNELS_16_BIT_AVX, d_begin, d_end, dseq_search_window );
                    }
                    else {
                        /* no more sequences, empty channel */

                        d_seq_ptr[c] = 0;
                        d_begin[c] = 0;
                        d_end[c] = d_begin[c];

                        for( int j = 0; j < CDEPTH_16_BIT; j++ )
                            dseq_search_window[CHANNELS_16_BIT_AVX * j + c] = 0;
                    }
                }
            }

            if( done == chunk->fill_pointer )
                break;

            dprofile_fill_16_avx2( dprofile, dseq_search_window );

            aligncolumns_first( &S.v, hep, s->queries[q_id]->q_table_avx, gap_open_extend, gap_extend, M.v, &h_max.v, qlen );
        }
        check_max( CHANNELS_16_BIT_AVX, overflow, h_max.a, score_max );

#ifdef DBG_COLLECT_MATRIX
        d_idx += 4;
#endif
    }

#ifdef DBG_COLLECT_MATRIX
        sequence * db_sequences = xmalloc( sizeof( sequence ) * done );

        for (int i = 0; i < done; ++i) {
            db_sequences[i] = chunk->seq[i]->seq;
        }

        dbg_print_matrices_to_file( BIT_WIDTH_16, "SW", s->queries[q_id]->seq, db_sequences, done );
#endif
}
