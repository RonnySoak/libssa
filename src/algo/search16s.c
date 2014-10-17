/*
 SWIPE
 Smith-Waterman database searches with Inter-sequence Parallel Execution

 Copyright (C) 2008-2013 Torbjorn Rognes, University of Oslo,
 Oslo University Hospital and Sencel Bioinformatics AS

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

#include <immintrin.h>

#include "util.h"
#include "../util_sequence.h"

//#define DEBUG

#define CHANNELS 8
#define CDEPTH 1

void dprofile_dump16s(uint16_t * dprofile)
{
    const char * s = sym_ncbi_aa;
    outf("dprofile_word:\n");
    for (int i = 0; i < 32; i++)
            {
        outf("%c: ", s[i]);
        for (int k = 0; k < CDEPTH; k++)
                {
            outf("[");
            for (int j = 0; j < CHANNELS; j++)
                outf("%2d",
                        (short) dprofile[CHANNELS * CDEPTH * i + CHANNELS * k
                                + j]);
            outf("]");
        }
        outf("\n");
    }
    exit(1);
}

// Register usage
// rdi:   hep
// rsi:   qp
// rdx:   Qm
// rcx:   Rm
// r8:    ql
// r9:    Sm/Mm

// rax:   x, temp
// r10:   ql2
// r11:   qi
// xmm0:  H0
// xmm1:  H1
// xmm2:  H2
// xmm3:  H3
// xmm4:  F0
// xmm5:  F1
// xmm6:  F2
// xmm7:  F3
// xmm8:  N0
// xmm9:  N1
// xmm10: N2
// xmm11: N3
// xmm12: E
// xmm13: S
// xmm14: Q
// xmm15: R

#define INITIALIZE			\
  "        movq    %0, %%rax        \n"	\
  "        movdqa  (%%rax), %%xmm13 \n"	\
  "        movdqa  (%3), %%xmm14    \n"	\
  "        movdqa  (%4), %%xmm15    \n"	\
  "        movq    %6, %%rax        \n"	\
  "        movdqa  (%%rax), %%xmm0  \n"	\
  "        movdqa  %%xmm0, %%xmm4   \n"	\
  "        shlq    $3, %5           \n"	\
  "        movq    %5, %%r10        \n"	\
  "        andq    $-16, %%r10      \n"	\
  "        xorq    %%r11, %%r11     \n"

#define ONESTEP(H, N, F, V)		\
  "        paddsw  "V"(%%rax), "H"  \n"	\
  "        pmaxsw  "F", "H"         \n"	\
  "        pmaxsw  %%xmm12, "H"     \n"	\
  "        pmaxsw  "H", %%xmm13     \n"	\
  "        psubsw  %%xmm15, "F"     \n"	\
  "        psubsw  %%xmm15, %%xmm12 \n"	\
  "        movdqa  "H", "N"         \n"	\
  "        psubsw  %%xmm14, "H"     \n"	\
  "        pmaxsw  "H", %%xmm12     \n"	\
  "        pmaxsw  "H", "F"         \n"

inline void donormal16s(volatile __m128i * Sm, /* r9  */
__m128i * hep, /* rdi */
__m128i ** qp, /* rsi */
__m128i * Qm, /* rdx */
__m128i * Rm, /* rcx */
long ql, /* r8  */
__m128i * Zm)
{
    __asm__
    __volatile__
    (
            "## donormal16s                            \n"
            INITIALIZE
            "        jmp     2f                      \n"

            "1:      movq    0(%2,%%r11,1), %%rax    \n" // load x from qp[qi]
            "        movdqa  0(%1,%%r11,4), %%xmm8   \n"// load N0
            "        movdqa  16(%1,%%r11,4), %%xmm12 \n"// load E

            ONESTEP("%%xmm0", "0(%1,%%r11,4)", "%%xmm4", "0" )

            "        movq    8(%2,%%r11,1), %%rax    \n"// load x from qp[qi+1]
            "        movdqa  %%xmm12, 16(%1,%%r11,4) \n"// save E
            "        movdqa  32(%1,%%r11,4), %%xmm0  \n"// load H0
            "        movdqa  48(%1,%%r11,4), %%xmm12 \n"// load E

            ONESTEP("%%xmm8", "32(%1,%%r11,4)", "%%xmm4", "0" )

            "        movdqa  %%xmm12, 48(%1,%%r11,4) \n"// save E
            "        addq    $16, %%r11              \n"// qi++
            "2:      cmpq    %%r11, %%r10            \n"// qi = ql4 ?
            "        jne     1b                      \n"// loop

            "        cmpq    %%r11, %5               \n"
            "        je      3f                      \n"
            "        movq    0(%2,%%r11,1), %%rax    \n"// load x from qp[qi]
            "        movdqa  16(%1,%%r11,4), %%xmm12 \n"// load E

            ONESTEP("%%xmm0", "0(%1,%%r11,4)", "%%xmm4", "0" )

            "        movdqa  %%xmm12, 16(%1,%%r11,4) \n"// save E
            "3:      movq    %0, %%rax               \n"// save S
            "        movdqa  %%xmm13, (%%rax)        \n"
            "        shrq    $3, %5                    "
            :
            : "m"(Sm), "r"(hep),"r"(qp),
            "r"(Qm), "r"(Rm), "r"(ql), "m"(Zm)
            : "xmm0", "xmm4", "xmm8",
            "xmm12", "xmm13", "xmm14", "xmm15",
            "rax", "r10", "r11", "cc",
            "memory"
    );
}

/* not used: 1 2 3 5 6 7 9 10 11 */
/* used 0 (H0) 4 (F0) 8 (N0) 12 (E) 13 (S) 14 (Q) 15 (R) */

inline void domasked16s(volatile __m128i * Sm,
        __m128i * hep,
        __m128i ** qp,
        __m128i * Qm,
        __m128i * Rm,
        long ql,
        __m128i * Zm,
        __m128i * Mm)
{
    __asm__
    __volatile__
    (
            "## domasked16                             \n"
            INITIALIZE
            "        paddsw  (%7), %%xmm13           \n" // add M
            "        jmp     2f                      \n"

            "1:      movq    0(%2,%%r11,1), %%rax    \n"// load x from qp[qi]
            "        movdqa  0(%1,%%r11,4), %%xmm8   \n"// load N0
            "        paddsw  (%7), %%xmm8            \n"// add M
            "        movdqa  16(%1,%%r11,4), %%xmm12 \n"// load E
            "        paddsw  (%7), %%xmm12           \n"// add M

            ONESTEP("%%xmm0", "0(%1,%%r11,4)", "%%xmm4", "0" )

            "        movdqa  %%xmm12, 16(%1,%%r11,4) \n"// save E
            "        movq    8(%2,%%r11,1), %%rax    \n"// load x from qp[qi+1]
            "        movdqa  32(%1,%%r11,4), %%xmm0  \n"// load H0
            "        paddsw  (%7), %%xmm0            \n"// add M
            "        movdqa  48(%1,%%r11,4), %%xmm12 \n"// load E
            "        paddsw  (%7), %%xmm12           \n"// add M

            ONESTEP("%%xmm8", "32(%1,%%r11,4)", "%%xmm4", "0" )

            "        movdqa  %%xmm12, 48(%1,%%r11,4) \n"// save E
            "        addq    $16, %%r11              \n"// qi++
            "2:      cmpq    %%r11, %%r10            \n"// qi = ql4 ?
            "        jne     1b                      \n"// loop

            "        cmpq    %%r11, %5               \n"
            "        je      3f                      \n"
            "        movq    0(%2,%%r11,1), %%rax    \n"// load x from qp[qi]
            "        movdqa  16(%1,%%r11,4), %%xmm12 \n"// load E
            "        paddsw  (%7), %%xmm12           \n"// add M

            ONESTEP("%%xmm0", "0(%1,%%r11,4)", "%%xmm4", "0" )

            "        movdqa  %%xmm12, 16(%1,%%r11,4) \n"// save E
            "3:      movq    %0, %%rax               \n"// save S
            "        movdqa  %%xmm13, (%%rax)        \n"
            "        shrq    $3, %5                    "
            :
            : "m"(Sm), "r"(hep),"r"(qp), "r"(Qm), "r"(Rm), "r"(ql), "m"(Zm),
            "r"(Mm)
            : "xmm0", "xmm4", "xmm8",
            "xmm12", "xmm13", "xmm14", "xmm15",
            "rax", "r10", "r11", "cc",
            "memory"
    );
}

inline void dprofile_fill16s(uint16_t * dprofile_word,
        uint16_t * score_matrix_word,
        uint8_t * dseq)
{
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    __m128i xmm16, xmm17, xmm18, xmm19, xmm20, xmm21, xmm22, xmm23;
    __m128i xmm24, xmm25, xmm26, xmm27, xmm28, xmm29, xmm30, xmm31;

    for (int j = 0; j < CDEPTH; j++)
            {
        int d[CHANNELS];
        for (int z = 0; z < CHANNELS; z++)
            d[z] = dseq[j * CHANNELS + z] << 5;

        //      for(int i=0; i<24; i += 8)
        for (int i = 0; i < 32; i += 8)
                {
            xmm0 = _mm_load_si128((__m128i *) (score_matrix_word + d[0] + i));
            xmm1 = _mm_load_si128((__m128i *) (score_matrix_word + d[1] + i));
            xmm2 = _mm_load_si128((__m128i *) (score_matrix_word + d[2] + i));
            xmm3 = _mm_load_si128((__m128i *) (score_matrix_word + d[3] + i));
            xmm4 = _mm_load_si128((__m128i *) (score_matrix_word + d[4] + i));
            xmm5 = _mm_load_si128((__m128i *) (score_matrix_word + d[5] + i));
            xmm6 = _mm_load_si128((__m128i *) (score_matrix_word + d[6] + i));
            xmm7 = _mm_load_si128((__m128i *) (score_matrix_word + d[7] + i));

            xmm8 = _mm_unpacklo_epi16(xmm0, xmm1);
            xmm9 = _mm_unpackhi_epi16(xmm0, xmm1);
            xmm10 = _mm_unpacklo_epi16(xmm2, xmm3);
            xmm11 = _mm_unpackhi_epi16(xmm2, xmm3);
            xmm12 = _mm_unpacklo_epi16(xmm4, xmm5);
            xmm13 = _mm_unpackhi_epi16(xmm4, xmm5);
            xmm14 = _mm_unpacklo_epi16(xmm6, xmm7);
            xmm15 = _mm_unpackhi_epi16(xmm6, xmm7);

            xmm16 = _mm_unpacklo_epi32(xmm8, xmm10);
            xmm17 = _mm_unpackhi_epi32(xmm8, xmm10);
            xmm18 = _mm_unpacklo_epi32(xmm12, xmm14);
            xmm19 = _mm_unpackhi_epi32(xmm12, xmm14);
            xmm20 = _mm_unpacklo_epi32(xmm9, xmm11);
            xmm21 = _mm_unpackhi_epi32(xmm9, xmm11);
            xmm22 = _mm_unpacklo_epi32(xmm13, xmm15);
            xmm23 = _mm_unpackhi_epi32(xmm13, xmm15);

            xmm24 = _mm_unpacklo_epi64(xmm16, xmm18);
            xmm25 = _mm_unpackhi_epi64(xmm16, xmm18);
            xmm26 = _mm_unpacklo_epi64(xmm17, xmm19);
            xmm27 = _mm_unpackhi_epi64(xmm17, xmm19);
            xmm28 = _mm_unpacklo_epi64(xmm20, xmm22);
            xmm29 = _mm_unpackhi_epi64(xmm20, xmm22);
            xmm30 = _mm_unpacklo_epi64(xmm21, xmm23);
            xmm31 = _mm_unpackhi_epi64(xmm21, xmm23);

            _mm_store_si128(
                    (__m128i *) (dprofile_word + CDEPTH * CHANNELS * (i + 0)
                            + CHANNELS * j), xmm24);
            _mm_store_si128(
                    (__m128i *) (dprofile_word + CDEPTH * CHANNELS * (i + 1)
                            + CHANNELS * j), xmm25);
            _mm_store_si128(
                    (__m128i *) (dprofile_word + CDEPTH * CHANNELS * (i + 2)
                            + CHANNELS * j), xmm26);
            _mm_store_si128(
                    (__m128i *) (dprofile_word + CDEPTH * CHANNELS * (i + 3)
                            + CHANNELS * j), xmm27);
            _mm_store_si128(
                    (__m128i *) (dprofile_word + CDEPTH * CHANNELS * (i + 4)
                            + CHANNELS * j), xmm28);
            _mm_store_si128(
                    (__m128i *) (dprofile_word + CDEPTH * CHANNELS * (i + 5)
                            + CHANNELS * j), xmm29);
            _mm_store_si128(
                    (__m128i *) (dprofile_word + CDEPTH * CHANNELS * (i + 6)
                            + CHANNELS * j), xmm30);
            _mm_store_si128(
                    (__m128i *) (dprofile_word + CDEPTH * CHANNELS * (i + 7)
                            + CHANNELS * j), xmm31);
        }
    }
    //  dprofile_dump16s(dprofile_word);
}

void search16s(uint16_t * * q_start,
        uint16_t gap_open_penalty,
        uint16_t gap_extend_penalty,
        uint16_t * score_matrix,
        uint16_t * dprofile,
        uint16_t * hearray,
        struct db_thread_s * * dbta,
        long sequences,
        long * seqnos,
        long * scores,
        long * bestpos,
        long * bestq,
        int qlen)
{
    volatile __m128i S;
    __m128i SL, Q, R, T, M, Z, T0;
    __m128i *hep, **qp;
    uint8_t * d_begin[CHANNELS];
    uint8_t * d_pos[CHANNELS];
    uint8_t * d_end[CHANNELS];
    uint8_t * d_best[CHANNELS];
    long q_best[CHANNELS];

    __m128i dseqalloc[CDEPTH];

    uint8_t * dseq = (uint8_t *) &dseqalloc;
    uint8_t zero;

    long seq_id[CHANNELS];
    long next_id = 0;
    unsigned done;

    Z = _mm_set_epi16(0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
            0x8000);
    T0 = _mm_set_epi16(0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
            0x8000);
    Q = _mm_set_epi16(gap_open_penalty, gap_open_penalty,
            gap_open_penalty, gap_open_penalty,
            gap_open_penalty, gap_open_penalty,
            gap_open_penalty, gap_open_penalty);
    R = _mm_set_epi16(gap_extend_penalty, gap_extend_penalty,
            gap_extend_penalty, gap_extend_penalty,
            gap_extend_penalty, gap_extend_penalty,
            gap_extend_penalty, gap_extend_penalty);

    zero = 0;
    done = 0;

    S = Z;
    SL = Z;

    hep = (__m128i *) hearray;
    qp = (__m128i **) q_start;

    for (int a = 0; a < qlen; a++)
            {
        hep[2 * a] = Z;
        hep[2 * a + 1] = Z;
    }

    for (int c = 0; c < CHANNELS; c++)
            {
        d_begin[c] = &zero;
        d_pos[c] = d_begin[c];
        d_end[c] = d_begin[c];
        d_best[c] = d_begin[c];
        q_best[c] = -1;
        seq_id[c] = -1;
    }

    int easy = 0;

    while (1)
    {
        if (easy)
        {
            for (int c = 0; c < CHANNELS; c++)
                    {
                for (int j = 0; j < CDEPTH; j++)
                        {
                    if (d_pos[c] < d_end[c])
                        dseq[CHANNELS * j + c] = *(d_pos[c]++);
                    else
                        dseq[CHANNELS * j + c] = 0;
                }
                if ((d_pos[c] == d_end[c]) && (seq_id[c] > -1))
                    easy = 0;
            }

            dprofile_fill16s(dprofile, score_matrix, dseq);

            donormal16s(&S, hep, qp, &Q, &R, qlen, &Z);

            /* save column address if new highscore */

            int mask = _mm_movemask_epi8(_mm_cmpgt_epi16((__m128i ) S, SL));
            if (mask)
            {
                for (int c = 0; c < CHANNELS; c++)
                    if (mask & (3 << 2 * c))
                        d_best[c] = d_pos[c] - 1;

                for (long i = qlen - 1; i >= 0; i--)
                        {
                    int m2 = mask
                            & _mm_movemask_epi8(
                                    _mm_cmpeq_epi16(hep[2 * i], (__m128i ) S));
                    if (m2)
                        for (int c = 0; c < CHANNELS; c++)
                            if (m2 & (3 << 2 * c))
                                q_best[c] = i;
                }
            }

#ifdef DEBUG
            outf("E mask=%04x\n", mask);
            outf("SL=");
            vector_print_word((uint16_t*)&SL);
            outf("\nS =");
            vector_print_word((uint16_t*)&S);
            outf("\nBe:");
            for (int c=0; c<CHANNELS; c++)
            outf(" %4ld", d_best[c]-d_begin[c]);
            outf("\n");
#endif

            SL = (__m128i ) S;
        }
        else
        {

            easy = 1;

            M = _mm_setzero_si128();
            T = T0;

            for (int c = 0; c < CHANNELS; c++)
                    {
                if (d_pos[c] < d_end[c])
                        {
                    for (int j = 0; j < CDEPTH; j++)
                            {
                        if (d_pos[c] < d_end[c])
                            dseq[CHANNELS * j + c] = *(d_pos[c]++);
                        else
                            dseq[CHANNELS * j + c] = 0;
                    }

                    if (d_pos[c] == d_end[c])
                        easy = 0;

                }
                else
                {
                    M = _mm_xor_si128(M, T);

                    long cand_id = seq_id[c];

                    if (cand_id >= 0)
                            {
                        long score = ((uint16_t*) &S)[c] ^ 0x8000;
                        scores[cand_id] = score;
                        bestpos[cand_id] = d_best[c] - d_begin[c];
                        bestq[cand_id] = q_best[c];
                        done++;
                    }

                    if (next_id < sequences)
                            {
                        seq_id[c] = next_id;
                        long seqnosf = seqnos[next_id];
                        char* address;
                        long length, ntlen;

                        long strand = (seqnosf >> 2) & 1;
                        long frame = seqnosf & 3;
                        long seqno = seqnosf >> 3;

                        db_mapsequences(dbta[c], seqno, seqno);

                        db_getsequence(dbta[c], seqno, strand, frame,
                                &address, &length, &ntlen, c);

                        d_begin[c] = (unsigned char*) address;
                        d_pos[c] = d_begin[c];
                        d_best[c] = d_begin[c];
                        d_end[c] = (unsigned char*) address + length - 1;
                        q_best[c] = -1;
                        next_id++;

                        for (int j = 0; j < CDEPTH; j++)
                                {
                            if (d_pos[c] < d_end[c])
                                dseq[CHANNELS * j + c] = *(d_pos[c]++);
                            else
                                dseq[CHANNELS * j + c] = 0;
                        }
                        if (d_pos[c] == d_end[c])
                            easy = 0;
                    }
                    else
                    {
                        seq_id[c] = -1;
                        d_pos[c] = &zero;
                        d_end[c] = d_pos[c];
                        for (int j = 0; j < CDEPTH; j++)
                            dseq[CHANNELS * j + c] = 0;
                    }
                }
                T = _mm_slli_si128(T, 2);
            }

            if (done == sequences)
                break;

            dprofile_fill16s(dprofile, score_matrix, dseq);

            domasked16s(&S, hep, qp, &Q, &R, qlen, &Z, &M);

            /* save column address if new highscore */

            SL = _mm_adds_epi16(SL, M);
            int mask = _mm_movemask_epi8(_mm_cmpgt_epi16((__m128i ) S, SL));
            if (mask)
            {
                for (int c = 0; c < CHANNELS; c++)
                    if (mask & (3 << 2 * c))
                        d_best[c] = d_pos[c] - 1;

                for (long i = qlen - 1; i >= 0; i--)
                        {
                    int m2 = mask
                            & _mm_movemask_epi8(
                                    _mm_cmpeq_epi16(hep[2 * i], (__m128i ) S));
                    if (m2)
                        for (int c = 0; c < CHANNELS; c++)
                            if (m2 & (3 << 2 * c))
                                q_best[c] = i;
                }
            }

#ifdef DEBUG
            outf("H mask=%04x\n", mask);
            outf("SL=");
            vector_print_word((uint16_t*)&SL);
            outf("\nS =");
            vector_print_word((uint16_t*)&S);
            outf("\nBe:");
            for (int c=0; c<CHANNELS; c++)
            outf(" %4ld", d_best[c]-d_begin[c]);
            outf("\n");
#endif

            SL = (__m128i ) S;
        }
    }
}
