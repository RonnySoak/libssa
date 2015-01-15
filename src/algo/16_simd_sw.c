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

#include "align_simd.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "../util/util_sequence.h"
#include "../util/util.h"
#include "../matrices.h"

#define CHANNELS 8
#define CDEPTH 4

/*
 * TODO
 * - change interface, to be the same, than the naive implementations
 * - integrate in the library
 */

/*
 Using 16-bit signed values, from 0 to +65535.
 match: positive
 mismatch: negative
 gap penalties: positive (open, extend)
 optimal local alignment (SW)
 maximize score
 */

static void _mm_print( char * desc, __m128i x ) {
    unsigned short * y = (unsigned short*) &x;

    printf( "%s: ", desc );

    for( int i = 0; i < 8; i++ )
        printf( "%s%6d", (i > 0 ? " " : ""), y[7 - i] );
    printf( "\n" );
}

static void _mm_print2( char * desc, __m128i x ) {
    signed short * y = (signed short*) &x;

    printf( "%s: ", desc );

    for( int i = 0; i < 8; i++ )
        printf( "%s%2d", (i > 0 ? " " : ""), y[7 - i] );
    printf( "\n" );
}

//static void dprofile_dump16( int16_t * dprofile ) {
//    const char * s = sym_ncbi_nt16u;
//    printf( "\ndprofile:\n" );
//    for( int i = 0; i < 16; i++ ) {
//        printf( "%c: ", s[i] );
//        for( int k = 0; k < CDEPTH; k++ ) {
//            printf( "[" );
//            for( int j = 0; j < CHANNELS; j++ )
//                printf( " %3d", dprofile[CHANNELS * CDEPTH * i + CHANNELS * k + j] );
//            printf( "]" );
//        }
//        printf( "\n" );
//    }
//}

//static void dumpscorematrix( int16_t * m ) {
//    for( int i = 0; i < 16; i++ ) {
//        printf( "%2d %c", i, sym_ncbi_nt16u[i] );
//        for( int j = 0; j < 16; j++ )
//            printf( " %2d", m[16 * i + j] );
//        printf( "\n" );
//    }
//}
static void dprofile_fill16( int16_t * dprofile_word, int16_t * score_matrix_word, uint8_t * dseq ) {
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    __m128i xmm16, xmm17, xmm18, xmm19, xmm20, xmm21, xmm22, xmm23;
    __m128i xmm24, xmm25, xmm26, xmm27, xmm28, xmm29, xmm30, xmm31;

    /* does not require ssse3 */
    /* approx 4*(5*8+2*40)=480 instructions */

#if 0
    dumpscorematrix(score_matrix_word);

    for (int j=0; j<CDEPTH; j++)
    {
        for(int z=0; z<CHANNELS; z++)
        fprintf(stderr, " [%c]", sym_ncbi_nt16u[dseq[j*CHANNELS+z]]);
        fprintf(stderr, "\n");
    }
#endif

    for( int j = 0; j < CDEPTH; j++ ) {
        int d[CHANNELS];
        for( int z = 0; z < CHANNELS; z++ )
            d[z] = dseq[j * CHANNELS + z] << 4;

        for( int i = 0; i < 16; i += 8 ) {
            xmm0 = _mm_load_si128( (__m128i *) (score_matrix_word + d[0] + i) );
            xmm1 = _mm_load_si128( (__m128i *) (score_matrix_word + d[1] + i) );
            xmm2 = _mm_load_si128( (__m128i *) (score_matrix_word + d[2] + i) );
            xmm3 = _mm_load_si128( (__m128i *) (score_matrix_word + d[3] + i) );
            xmm4 = _mm_load_si128( (__m128i *) (score_matrix_word + d[4] + i) );
            xmm5 = _mm_load_si128( (__m128i *) (score_matrix_word + d[5] + i) );
            xmm6 = _mm_load_si128( (__m128i *) (score_matrix_word + d[6] + i) );
            xmm7 = _mm_load_si128( (__m128i *) (score_matrix_word + d[7] + i) );

            xmm8 = _mm_unpacklo_epi16( xmm0, xmm1 );
            xmm9 = _mm_unpackhi_epi16( xmm0, xmm1 );
            xmm10 = _mm_unpacklo_epi16( xmm2, xmm3 );
            xmm11 = _mm_unpackhi_epi16( xmm2, xmm3 );
            xmm12 = _mm_unpacklo_epi16( xmm4, xmm5 );
            xmm13 = _mm_unpackhi_epi16( xmm4, xmm5 );
            xmm14 = _mm_unpacklo_epi16( xmm6, xmm7 );
            xmm15 = _mm_unpackhi_epi16( xmm6, xmm7 );

            xmm16 = _mm_unpacklo_epi32( xmm8, xmm10 );
            xmm17 = _mm_unpackhi_epi32( xmm8, xmm10 );
            xmm18 = _mm_unpacklo_epi32( xmm12, xmm14 );
            xmm19 = _mm_unpackhi_epi32( xmm12, xmm14 );
            xmm20 = _mm_unpacklo_epi32( xmm9, xmm11 );
            xmm21 = _mm_unpackhi_epi32( xmm9, xmm11 );
            xmm22 = _mm_unpacklo_epi32( xmm13, xmm15 );
            xmm23 = _mm_unpackhi_epi32( xmm13, xmm15 );

            xmm24 = _mm_unpacklo_epi64( xmm16, xmm18 );
            xmm25 = _mm_unpackhi_epi64( xmm16, xmm18 );
            xmm26 = _mm_unpacklo_epi64( xmm17, xmm19 );
            xmm27 = _mm_unpackhi_epi64( xmm17, xmm19 );
            xmm28 = _mm_unpacklo_epi64( xmm20, xmm22 );
            xmm29 = _mm_unpackhi_epi64( xmm20, xmm22 );
            xmm30 = _mm_unpacklo_epi64( xmm21, xmm23 );
            xmm31 = _mm_unpackhi_epi64( xmm21, xmm23 );

            _mm_store_si128( (__m128i *) (dprofile_word +
            CDEPTH * CHANNELS * (i + 0) + CHANNELS * j), xmm24 );
            _mm_store_si128( (__m128i *) (dprofile_word +
            CDEPTH * CHANNELS * (i + 1) + CHANNELS * j), xmm25 );
            _mm_store_si128( (__m128i *) (dprofile_word +
            CDEPTH * CHANNELS * (i + 2) + CHANNELS * j), xmm26 );
            _mm_store_si128( (__m128i *) (dprofile_word +
            CDEPTH * CHANNELS * (i + 3) + CHANNELS * j), xmm27 );
            _mm_store_si128( (__m128i *) (dprofile_word +
            CDEPTH * CHANNELS * (i + 4) + CHANNELS * j), xmm28 );
            _mm_store_si128( (__m128i *) (dprofile_word +
            CDEPTH * CHANNELS * (i + 5) + CHANNELS * j), xmm29 );
            _mm_store_si128( (__m128i *) (dprofile_word +
            CDEPTH * CHANNELS * (i + 6) + CHANNELS * j), xmm30 );
            _mm_store_si128( (__m128i *) (dprofile_word +
            CDEPTH * CHANNELS * (i + 7) + CHANNELS * j), xmm31 );
        }
    }
#if 0
    dprofile_dump16(dprofile_word);
#endif
}

/*
 * TODO Can we mix epi and epu intrinsics?
 */

#define ALIGNCORE(H, N, F, V, QR, R, S)                                        \
 H = _mm_adds_epi16(H, V);            /* add value of scoring matrix */        \
 H = _mm_max_epi16(H, F);             /* max(H, F) */                          \
 H = _mm_max_epi16(H, E);             /* max(H, E) */                          \
 S = _mm_max_epi16(H, S);             /* save max score */                     \
 N = H;                               /* save H in HE-array */                 \
 HF = _mm_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 F = _mm_subs_epi16(F, R);            /* subtract gap extend */                \
 F = _mm_max_epi16(F, HF);            /* test for gap extension, or opening */ \
 HE = _mm_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 E = _mm_subs_epi16(E, R);            /* subtract gap extend */                \
 E = _mm_max_epi16(E, HE);            /* test for gap extension, or opening */

static void aligncolumns_first( __m128i * Sm, __m128i * hep, __m128i ** qp, __m128i gap_open_extend, __m128i gap_extend,
        __m128i h0, __m128i h1, __m128i h2, __m128i h3, __m128i Mm,
        long ql ) {
    __m128i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m128i * vp;
    long i;

    f0 = f1 = f2 = f3 = _mm_setzero_si128();

//    _mm_print2( "M_QR_t_left", M_QR_t_left );

    for( i = 0; i < ql - 1; i++ ) {
//    printf("round: %d\n", i);
        vp = qp[i + 0];

//    _mm_print2( "vp 0", vp[0] );
//    _mm_print2( "vp 1", vp[1] );
//    _mm_print2( "vp 2", vp[2] );
//    _mm_print2( "vp 3", vp[3] );

        h4 = hep[2 * i + 0];
//        _mm_print2( "h4", h4 );

        h4 = _mm_subs_epu16( h4, Mm );
//        _mm_print2( "h4", h4 );

//        h4 = _mm_subs_epu16( h4, M_QR_t_left );
//        _mm_print2( "h4", h4 );


        E = hep[2 * i + 1];
//        _mm_print2( "E", E );

        E = _mm_subs_epu16( E, Mm );
//        _mm_print2( "E", E );

//        E = _mm_adds_epi16( E, E0 );
//        _mm_print2( "E", E );

//        _mm_print2( "h0", h0 );
//        _mm_print2( "h1", h1 );
//        _mm_print2( "h2", h2 );
//        _mm_print2( "h3", h3 );

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm );
//        _mm_print2( "S", *Sm );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm );
//        _mm_print2( "S", *Sm );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm );
//        _mm_print2( "S", *Sm );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm );
//        _mm_print2( "S", *Sm );

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;

//        _mm_print2( "S", *Sm );
//        _mm_print2( "h0", h0 );
//        _mm_print2( "h1", h1 );
//        _mm_print2( "h2", h2 );
//        _mm_print2( "h3", h3 );

    }
//        exit(1);

    /* the final round - using alternative query gap penalties */

    vp = qp[i + 0];

    E = hep[2 * i + 1];
    E = _mm_subs_epu16( E, Mm );

    ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm );
    ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm );
    ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm );
    ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm );

    hep[2 * i + 0] = h8;
    hep[2 * i + 1] = E;
}

static void aligncolumns_rest( __m128i * Sm, __m128i * hep, __m128i ** qp, __m128i gap_open_extend, __m128i gap_extend,
        __m128i h0, __m128i h1, __m128i h2, __m128i h3, long ql ) {
    __m128i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m128i * vp;
    long i;

    f0 = f1 = f2 = f3 = _mm_setzero_si128();
 //   h0 = h1 = h2 = h3 = _mm_setzero_si128();

    for( i = 0; i < ql - 1; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm );

        hep[2 * i + 0] = h8;
        hep[2 * i + 1] = E;

        h0 = h4;
        h1 = h5;
        h2 = h6;
        h3 = h7;
    }

    /* the final round - using alternative query gap penalties */

    vp = qp[i + 0];

    E = hep[2 * i + 1];

    ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend, *Sm );
    ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend, *Sm );
    ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend, *Sm );
    ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend, *Sm );

    hep[2 * i + 0] = h8;
    hep[2 * i + 1] = E;
}

static inline void fill_channel( int c, uint8_t* d_begin[CHANNELS], uint8_t* d_end[CHANNELS], uint8_t* dseq ) {
    /* fill channel */
    for( int j = 0; j < CDEPTH; j++ ) {
        if( d_begin[c] < d_end[c] ) {
            dseq[CHANNELS * j + c] = *(d_begin[c]++);
        }
        else {
            dseq[CHANNELS * j + c] = 0;
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

void search16_sw( p_s16info s, p_db_chunk chunk, p_minheap heap, int query_id ) {
    int16_t * dprofile = (int16_t*) s->dprofile;
    int16_t * hearray = (int16_t*) s->hearray;
    unsigned long qlen = s->qlen;

    __m128i T, M, T0;

    __m128i gap_open_extend, gap_extend;

    __m128i *hep;

    uint8_t * d_begin[CHANNELS];
    uint8_t * d_end[CHANNELS];
    long seq_id[CHANNELS];

    __m128i dseqalloc[CDEPTH];

    union {
        __m128i v;
        int16_t a[sizeof(__m128i )];
    } S;

    uint8_t * dseq = (uint8_t*) &dseqalloc;
    uint8_t zero = 0;

    unsigned long next_id = 0;
    unsigned long done = 0;

    T0 = _mm_set_epi16( 0, 0, 0, 0, 0, 0, 0, 0xffff );
    S.v = _mm_setzero_si128();

    gap_open_extend = _mm_set1_epi16( s->penalty_gap_open + s->penalty_gap_extension );
    gap_extend = _mm_set1_epi16( s->penalty_gap_extension );

    hep = (__m128i *) hearray;

    for( int c = 0; c < CHANNELS; c++ ) {
        d_begin[c] = &zero;
        d_end[c] = d_begin[c];
        seq_id[c] = -1;
    }

    for( int i = 0; i < 4; i++ ) {
        dseqalloc[i] = _mm_setzero_si128();
    }

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
            _mm_print( "Score 2", S.v );

            /* fill all channels with symbols from the database sequences */

            for( int c = 0; c < CHANNELS; c++ ) {
                fill_channel( c, d_begin, d_end, dseq );

                if( d_begin[c] == d_end[c] )
                    no_sequences_ended = 0;
            }

            dprofile_fill16( dprofile, (int16_t*) s->matrix, dseq );

            aligncolumns_rest( &S.v, hep, s->qtable, gap_open_extend, gap_extend, H0, H1, H2, H3, qlen );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            no_sequences_ended = 1;

            M = _mm_setzero_si128();
            T = T0;

            _mm_print( "Score 1", S.v );

            for( int c = 0; c < CHANNELS; c++ ) {
                if( d_begin[c] < d_end[c] ) {
                    /* this channel has more sequence */

                    fill_channel( c, d_begin, d_end, dseq );

                    if( d_begin[c] == d_end[c] )
                        no_sequences_ended = 0;
                }
                else {
                    /* sequence in channel c ended. change of sequence */

                    M = _mm_xor_si128( M, T );

                    long cand_id = seq_id[c];

                    if( cand_id >= 0 ) {
                        /* save score */

                        long score = S.a[c];

                        if( (score >= 0) && (score < UINT16_MAX) ) {
                            /* Alignments, with a score equal to the current lowest score in the
                             heap are ignored! */
                            add_to_minheap( heap, query_id, chunk->seq[next_id - 1], score );
                        }
                        else {
                            // TODO else report recalculation
                            printf("\n\nscore out of range: %ld\n\n", score);
                        }

                        done++;
                    }

                    if( next_id < chunk->size ) {
                        char* address;
                        long length = 0; // TODO do we need this check for length > 0 ?

                        /* get next sequence with length>0 */

                        while( (length == 0) && (next_id < chunk->size) ) {
                            seq_id[c] = next_id;

                            address = chunk->seq[next_id]->seq.seq;
                            length = chunk->seq[next_id]->seq.len;

                            next_id++;
                        }

                        d_begin[c] = (unsigned char*) address;
                        d_end[c] = (unsigned char*) address + length;

                        fill_channel( c, d_begin, d_end, dseq );

                        if( d_begin[c] == d_end[c] )
                            no_sequences_ended = 0;
                    }
                    else {
                        /* no more sequences, empty channel */

                        seq_id[c] = -1;
                        d_begin[c] = &zero;
                        d_end[c] = d_begin[c];

                        for( int j = 0; j < CDEPTH; j++ )
                            dseq[CHANNELS * j + c] = 0;
                    }
                }

                T = _mm_slli_si128( T, 2 );
            }

            if( done == chunk->size )
                break;

            dprofile_fill16( dprofile, (int16_t *) s->matrix, dseq );

            aligncolumns_first( &S.v, hep, s->qtable, gap_open_extend, gap_extend, H0, H1, H2, H3, M,
                    qlen );
        }
    }
}
