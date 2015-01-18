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
#include <stdio.h>
#include <string.h>

#include "../../util/util_sequence.h"
#include "../../util/util.h"
#include "../../matrices.h"

#define CHANNELS 8
#define CDEPTH 4

/*
 * TODO
 * - change interface, to be the same, than the naive implementations
 * - integrate in the library
 */

/*
 Using 16-bit signed values, from -32768 to +32767.
 match: positive
 mismatch: negative
 gap penalties: positive (open, extend)
 optimal global alignment (NW)
 maximize score
 */

//static void _mm_print( char * desc, __m128i x ) {
//    unsigned short * y = (unsigned short*) &x;
//
//    printf( "%s: ", desc );
//
//    for( int i = 0; i < 8; i++ )
//        printf( "%s%6d", (i > 0 ? " " : ""), y[7 - i] );
//    printf( "\n" );
//}

//static void _mm_print2( char * desc, __m128i x ) {
//    signed short * y = (signed short*) &x;
//
//    printf( "%s: ", desc );
//
//    for( int i = 0; i < 8; i++ )
//        printf( "%s%2d", (i > 0 ? " " : ""), y[7 - i] );
//    printf( "\n" );
//}

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
 * TODO detect, if the score goes below INT16_MIN or above INT16_MAX in between the calculation
 */

#define ALIGNCORE(H, N, F, V, QR, R)                                           \
 H = _mm_adds_epi16(H, V);            /* add value of scoring matrix */        \
 H = _mm_max_epi16(H, F);             /* max(H, F) */                          \
 H = _mm_max_epi16(H, E);             /* max(H, E) */                          \
 N = H;                               /* save H in HE-array */                 \
 HF = _mm_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 F = _mm_subs_epi16(F, R);            /* subtract gap extend */                \
 F = _mm_max_epi16(F, HF);            /* test for gap extension, or opening */ \
 HE = _mm_subs_epi16(H, QR);          /* subtract gap open-extend */           \
 E = _mm_subs_epi16(E, R);            /* subtract gap extend */                \
 E = _mm_max_epi16(E, HE);            /* test for gap extension, or opening */

static void aligncolumns_first( __m128i * Sm, __m128i * hep, __m128i ** qp, __m128i gap_open_extend, __m128i gap_extend,
        __m128i h0, __m128i h1, __m128i h2, __m128i h3, __m128i E0, __m128i Mm, __m128i M_QR_t_left, __m128i M_R_t_left,
        long ql ) {
    __m128i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m128i * vp;
    long i;

    f0 = f1 = f2 = f3 = _mm_set1_epi16( SHRT_MIN );

    for( i = 0; i < ql - 1; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];
        h4 = _mm_subs_epu16( h4, Mm );
        h4 = _mm_subs_epi16( h4, M_QR_t_left );
        M_QR_t_left = _mm_adds_epi16( M_QR_t_left, M_R_t_left );

        E = hep[2 * i + 1];
        E = _mm_subs_epu16( E, Mm ); // sets new beginning columns to 0
        E = _mm_adds_epi16( E, E0 ); // sets new beginning columns to -32768
        // because we have to open an gap, and cannot extend it, since we are beginning here

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend );

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
    E = _mm_subs_epu16( E, Mm );
    E = _mm_adds_epi16( E, E0 );

    ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend );
    ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend );
    ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend );
    ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend );

    hep[2 * i + 0] = h8;
    hep[2 * i + 1] = E;

    Sm[0] = h5;
    Sm[1] = h6;
    Sm[2] = h7;
    Sm[3] = h8;
}

static void aligncolumns_rest( __m128i * Sm, __m128i * hep, __m128i ** qp, __m128i gap_open_extend, __m128i gap_extend,
        __m128i h0, __m128i h1, __m128i h2, __m128i h3, long ql ) {
    __m128i h4, h5, h6, h7, h8, f0, f1, f2, f3, E, HE, HF;
    __m128i * vp;
    long i;

    f0 = f1 = f2 = f3 = _mm_set1_epi16( SHRT_MIN );

    for( i = 0; i < ql - 1; i++ ) {
        vp = qp[i + 0];

        h4 = hep[2 * i + 0];

        E = hep[2 * i + 1];

        ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend );
        ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend );
        ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend );
        ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend );

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

    ALIGNCORE( h0, h5, f0, vp[0], gap_open_extend, gap_extend );
    ALIGNCORE( h1, h6, f1, vp[1], gap_open_extend, gap_extend );
    ALIGNCORE( h2, h7, f2, vp[2], gap_open_extend, gap_extend );
    ALIGNCORE( h3, h8, f3, vp[3], gap_open_extend, gap_extend );

    hep[2 * i + 0] = h8;
    hep[2 * i + 1] = E;

    Sm[0] = h5;
    Sm[1] = h6;
    Sm[2] = h7;
    Sm[3] = h8;
}

p_s16info search16_init( int16_t penalty_gap_open, int16_t penalty_gap_extension ) {
    /* prepare alloc of qtable, dprofile, hearray, dir */
    p_s16info s = (p_s16info) xmalloc( sizeof(struct s16info) );

    s->maxdlen = 4 * ((ssa_db_get_longest_sequence() + 3) / 4);
    s->dprofile = (__m128i *) xmalloc( 2 * 4 * 8 * 16 );
    s->qlen = 0;
    s->maxqlen = 0;
    s->hearray = 0;
    s->qtable = 0;

    for( int i = 0; i < 16; i++ ) {
        for( int j = 0; j < 16; j++ ) {
            ((int16_t*) (&s->matrix))[16 * i + j] = SCORE_MATRIX_16( i, j );
        }
    }

    s->penalty_gap_open = penalty_gap_open;
    s->penalty_gap_extension = penalty_gap_extension;

    return s;
}

void search16_exit( p_s16info s ) {
    /* free mem for dprofile, hearray, dir, qtable */
    if( s->hearray )
        free( s->hearray );
    if( s->dprofile )
        free( s->dprofile );
    if( s->qtable )
        free( s->qtable );
    free( s );
}

void search16_init_query( p_s16info s, char * qseq, int qlen ) {
    s->qlen = qlen;

    if( s->qlen > s->maxqlen ) {
        if( s->maxqlen == 0 )
            s->maxqlen = 256;
        while( s->qlen > s->maxqlen )
            s->maxqlen <<= 1;

        if( s->hearray )
            free( s->hearray );
        s->hearray = (__m128i *) xmalloc( 2 * s->maxqlen * sizeof(__m128i ) );
        memset( s->hearray, 0, 2 * s->maxqlen * sizeof(__m128i ) );

        if( s->qtable )
            free( s->qtable );
        s->qtable = (__m128i **) xmalloc( s->maxqlen * sizeof(__m128i *) );
    }

    for( int i = 0; i < qlen; i++ )
        s->qtable[i] = s->dprofile + 4 * (int) (qseq[i]);
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

void search16_nw( p_s16info s, p_db_chunk chunk, p_minheap heap, int query_id ) {
    int16_t * dprofile = (int16_t*) s->dprofile;
    unsigned long qlen = s->qlen;

    __m128i T, M, T0, E0;

    __m128i M_QR_target_left, M_R_target_left;

    __m128i gap_open_extend, gap_extend;

    union {
        __m128i * v;
        int16_t * a;
    } hep;

    uint8_t * d_begin[CHANNELS];
    uint8_t * d_end[CHANNELS];
    unsigned long d_length[CHANNELS];
    long seq_id[CHANNELS];

    __m128i dseqalloc[CDEPTH];
    __m128i S[4];

    uint8_t * dseq = (uint8_t*) &dseqalloc;
    uint8_t zero = 0;

    unsigned long next_id = 0;
    unsigned long done = 0;

    T0 = _mm_set_epi16( 0, 0, 0, 0, 0, 0, 0, 0xffff );

    gap_open_extend = _mm_set1_epi16( s->penalty_gap_open + s->penalty_gap_extension );
    gap_extend = _mm_set1_epi16( s->penalty_gap_extension );

    hep.a = (int16_t*) s->hearray; // TODO why do we go the detour from _m128i * to int16_t * and back?

    for( int c = 0; c < CHANNELS; c++ ) {
        d_begin[c] = &zero;
        d_end[c] = d_begin[c];
        d_length[c] = 0;
        seq_id[c] = -1;
    }

    for( int i = 0; i < 4; i++ ) {
        S[i] = _mm_setzero_si128();
        dseqalloc[i] = _mm_setzero_si128();
    }

    __m128i VECTOR_INT16_MIN = _mm_set1_epi16( INT16_MIN );
    __m128i H0 = _mm_setzero_si128();
    __m128i H1 = _mm_setzero_si128();
    __m128i H2 = _mm_setzero_si128();
    __m128i H3 = _mm_setzero_si128();

    int no_sequences_ended = 0;

    /*
     * TODO convert infinite loop into a loop with a condition
     */
    while( 1 ) {
        if( no_sequences_ended ) {
            /* fill all channels with symbols from the database sequences */

            for( int c = 0; c < CHANNELS; c++ ) {
                fill_channel( c, d_begin, d_end, dseq );

                if( d_begin[c] == d_end[c] )
                    no_sequences_ended = 0;
            }

            dprofile_fill16( dprofile, (int16_t*) s->matrix, dseq );

            aligncolumns_rest( S, hep.v, s->qtable, gap_open_extend, gap_extend, H0, H1, H2, H3, qlen );
        }
        else {
            /* One or more sequences ended in the previous block.
             We have to switch over to a new sequence           */
            no_sequences_ended = 1;

            M = _mm_setzero_si128();
            T = T0;
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

                        long z = (d_length[c] + 3) % 4;
                        long score = ((int16_t*) S)[z * CHANNELS + c];

                        if( (score > INT16_MIN) && (score < INT16_MAX) ) {
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

                        d_length[c] = length;
                        d_begin[c] = (unsigned char*) address;
                        d_end[c] = (unsigned char*) address + length;

                        ((int16_t*) &H0)[c] = 0;
                        ((int16_t*) &H1)[c] = -s->penalty_gap_open - 1 * s->penalty_gap_extension;
                        ((int16_t*) &H2)[c] = -s->penalty_gap_open - 2 * s->penalty_gap_extension;
                        ((int16_t*) &H3)[c] = -s->penalty_gap_open - 3 * s->penalty_gap_extension;

                        fill_channel( c, d_begin, d_end, dseq );

                        if( d_begin[c] == d_end[c] )
                            no_sequences_ended = 0;
                    }
                    else {
                        /* no more sequences, empty channel */

                        seq_id[c] = -1;
                        d_begin[c] = &zero;
                        d_end[c] = d_begin[c];
                        d_length[c] = 0;
                        for( int j = 0; j < CDEPTH; j++ )
                            dseq[CHANNELS * j + c] = 0;
                    }
                }

                T = _mm_slli_si128( T, 2 );
            }

            if( done == chunk->size )
                break;

            /* make masked versions of QR, R and E0 */
            M_QR_target_left = _mm_and_si128( M, gap_open_extend );
            M_R_target_left = _mm_and_si128( M, gap_extend );
            E0 = _mm_and_si128( M, VECTOR_INT16_MIN );

            dprofile_fill16( dprofile, (int16_t *) s->matrix, dseq );

            aligncolumns_first( S, hep.v, s->qtable, gap_open_extend, gap_extend, H0, H1, H2, H3, E0, M,
                    M_QR_target_left, M_R_target_left, qlen );
        }

        H0 = _mm_subs_epi16( H3, gap_extend );
        H1 = _mm_subs_epi16( H0, gap_extend );
        H2 = _mm_subs_epi16( H1, gap_extend );
        H3 = _mm_subs_epi16( H2, gap_extend );
    }
}
