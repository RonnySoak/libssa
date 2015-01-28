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

#include <limits.h>

#include "../../util/util.h"
#include "../../matrices.h"

#define ALIGNCORE(H, N, F, V, QR, R, S, H_MAX )                         \
 H = _mm_adds_epi16(H, V);            /* add value of scoring matrix */        \
 H = _mm_max_epi16(H, F);             /* max(H, F) */                          \
 H = _mm_max_epi16(H, E);             /* max(H, E) */                          \
 S = _mm_max_epi16(H, S);             /* save max score */                     \
 H = _mm_max_epi16(H, _mm_setzero_si128());          /* TODO do not call _mm_setzero_si128 every time*/ \
 H_MAX = _mm_max_epi16(H_MAX, H);                                              \
 N = H;                               /* save H in HE-array */                 \
 HF = _mm_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 F = _mm_subs_epi16(F, R);            /* subtract gap extend */                \
 F = _mm_max_epi16(F, HF);            /* test for gap extension, or opening */ \
 HE = _mm_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 E = _mm_subs_epi16(E, R);            /* subtract gap extend */                \
 E = _mm_max_epi16(E, HE);            /* test for gap extension, or opening */

static void aligncolumns_first( __m128i * Sm, __m128i * hep, __m128i ** qp, __m128i gap_open_extend, __m128i gap_extend,
        __m128i h0, __m128i h1, __m128i h2, __m128i h3, __m128i Mm, __m128i * _h_max, long ql ) {
    __m128i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m128i * vp;
    __m128i h_max = _mm_setzero_si128();
    long i;

    f0 = f1 = f2 = f3 = _mm_setzero_si128();

    for( i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];
        h4 = _mm_subs_epu16( h4, Mm ); // sets h4 to zero if a sequence starts new
        // h4 is zero, since it is at the beginning of a database sequence

        E = hep[2 * i + 1];
        E = _mm_subs_epu16( E, Mm ); // sets E to zero if a sequence starts new
        // Mm is initialized with 0xffff for all database sequences, that start new

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm, h_max );

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
        __m128i h0, __m128i h1, __m128i h2, __m128i h3,  __m128i * _h_max, long ql ) {
    __m128i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m128i * vp;
    __m128i h_max = _mm_setzero_si128();
    long i;

    f0 = f1 = f2 = f3 = _mm_setzero_si128();

    for( i = 0; i < ql; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm, h_max );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm, h_max );

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;
    }

    *_h_max = h_max;
}

static void check_min_max( uint8_t overflow[CHANNELS], __m128i h_max, int16_t score_max ) {
    for( int c = 0; c < CHANNELS; c++ ) {
        if( !overflow[c] ) {
            int16_t h_max_array[8];
            _mm_storeu_si128( (__m128i *) h_max_array, h_max );
            int16_t h_max_c = h_max_array[c];
            if( h_max_c >= score_max ) {
                printf("h_max: %d\n", h_max_c);
                overflow[c] = 1;
            }
        }
    }
}

/*
 * use the range from -32768 - 0 - to 32767
 *
 * init with int16_min and add int16_max to max
 *
 * add -32768 twice, to reset to zero (modify masking values)
 *
 * a biased Version adds more instructions in ALIGNCORE, than the other version
 */

void search_16_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, int q_id ) {
    int16_t * dprofile = (int16_t*) s->dprofile;
    unsigned long qlen = s->queries[q_id]->q_len;

    __m128i T, M, T0;

    __m128i gap_open_extend, gap_extend;

    __m128i * hep;

    uint8_t * d_begin[CHANNELS];
    uint8_t * d_end[CHANNELS];
    p_sdb_sequence d_seq_ptr[CHANNELS];
    uint8_t overflow[CHANNELS];

    __m128i dseqalloc[CDEPTH];

    union {
        __m128i v;
        int16_t a[sizeof(__m128i )];
    } S;

    uint8_t * dseq = (uint8_t*) &dseqalloc;

    unsigned long next_id = 0;
    unsigned long done = 0;

    T0 = _mm_set_epi16( 0, 0, 0, 0, 0, 0, 0, 0xffff );
    S.v = _mm_setzero_si128();

    hep = s->hearray;

    gap_open_extend = _mm_set1_epi16( s->penalty_gap_open + s->penalty_gap_extension );
    gap_extend = _mm_set1_epi16( s->penalty_gap_extension );

    for( int c = 0; c < CHANNELS; c++ ) {
        d_begin[c] = 0;
        d_end[c] = d_begin[c];
        d_seq_ptr[c] = 0;
        overflow[c] = 0;
    }

    for( int i = 0; i < 4; i++ ) {
        dseqalloc[i] = _mm_setzero_si128();
    }

    int16_t score_max = INT16_MAX;

    __m128i H0 = _mm_setzero_si128();
    __m128i H1 = _mm_setzero_si128();
    __m128i H2 = _mm_setzero_si128();
    __m128i H3 = _mm_setzero_si128();

    int no_sequences_ended = 0;

    /*
     * TODO convert infinite loop into a loop with a condition
     */
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

            for( int c = 0; c < CHANNELS; c++ ) {
                no_sequences_ended = fill_channel( c, d_begin, d_end, dseq );
            }

            dprofile_fill16( dprofile, dseq );

            __m128i h_max;

            aligncolumns_rest( &S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, H0, H1, H2, H3,
                    &h_max, qlen );

            check_min_max( overflow, h_max, score_max );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            no_sequences_ended = 1;

            M = _mm_setzero_si128();
            T = T0;
            for( int c = 0; c < CHANNELS; c++ ) {
                if( d_begin[c] < d_end[c] ) {
                    /* the sequence in this channel is not finished yet */

                    no_sequences_ended = fill_channel( c, d_begin, d_end, dseq );
                }
                else {
                    /* sequence in channel c ended. change of sequence */

                    M = _mm_xor_si128( M, T );

                    if( d_seq_ptr[c] ) {
                        /* save score */

                        long score = S.a[c];

                        if( overflow[c] ) {
                            printf( "\nWARNING! Alignment overflow!\n" );
                            printf( "Seqid: %lu, score: %ld\n", d_seq_ptr[c]->ID, score );

                            printf("overflow[c]: %d\n", overflow[c]);
                            overflow[c] = 0;
                        }

                        if( (score >= 0) && (score < UINT16_MAX) ) {
                            /* Alignments, with a score equal to the current lowest score in the
                             heap are ignored! */
                            add_to_minheap( heap, q_id, d_seq_ptr[c], score );
                        }
                        else {
                            // TODO else report recalculation
                            printf( "\n\nscore out of range: %ld\n\n", score );
                        }

                        done++;
                    }

                    if( next_id < chunk->fill_pointer ) {
                        char* address;
                        long length = 0; // TODO do we need this check for length > 0 ? Without it, the speed drops drastically ...

                        /* get next sequence with length>0 */

                        while( (length == 0) && (next_id < chunk->fill_pointer) ) {
                            d_seq_ptr[c] = chunk->seq[next_id];

                            address = d_seq_ptr[c]->seq.seq;
                            length = d_seq_ptr[c]->seq.len;

                            next_id++;
                        }

                        d_begin[c] = (unsigned char*) address;
                        d_end[c] = (unsigned char*) address + length;

                        no_sequences_ended = fill_channel( c, d_begin, d_end, dseq );
                    }
                    else {
                        /* no more sequences, empty channel */

                        d_seq_ptr[c] = 0;
                        d_begin[c] = 0;
                        d_end[c] = d_begin[c];

                        for( int j = 0; j < CDEPTH; j++ )
                            dseq[CHANNELS * j + c] = 0;
                    }
                }

                T = _mm_slli_si128( T, 2 );
            }

            if( done == chunk->fill_pointer )
                break;

            dprofile_fill16( dprofile, dseq );

            __m128i h_max;

            aligncolumns_first( &S.v, hep, s->queries[q_id]->q_table, gap_open_extend, gap_extend, H0, H1, H2, H3, M,
                    &h_max, qlen );

            check_min_max( overflow, h_max, score_max );
        }
    }
}
