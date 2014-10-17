/*
 * align_new.c
 *
 *  Created on: Oct 15, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "align.h"

#include <stdlib.h>

#include "../util.h"
#include "searcher.h"
#include "../matrices.h"

/*
 * TODO merge finding region and finding directions
 */
region_t find_region_and_score_for_local(sequence a_seq, sequence b_seq) {
    region_t region;

    long * HH = xmalloc(a_seq.len * sizeof(long));
    long * EE = xmalloc(a_seq.len * sizeof(long));

    long score = 0;

    // Forward pass
    for (long j = 0; j < a_seq.len; j++) {
        HH[j] = 0;
        EE[j] = -gapO;
    }

    for (long i = 0; i < b_seq.len; i++) {
        long h = 0;
        long p = 0;
        long f = -gapO;
        for (long j = 0; j < a_seq.len; j++) {
            f = MAX(f, h - gapO) - gapE;
            EE[j] = MAX(EE[j], HH[j] - gapO) - gapE;

            h = p + SCORE_MATRIX_63(a_seq.seq[j], b_seq.seq[i]);

            if (h < 0)
                h = 0;
            if (f > h)
                h = f;
            if (EE[j] > h)
                h = EE[j];

            p = HH[j];

            HH[j] = h;

            if (h > score) {
                score = h;
                region.a_end = j;
                region.b_end = i;
            }
        }
    }

    // Reverse pass
    for (long j = region.b_end; j >= 0; j--) {
        HH[j] = -1;
        EE[j] = -1;
    }

    long cost = 0;

    for (long i = region.a_end; i >= 0; i--) {
        long h = -1;
        long f = -1;
        long p;
        if (i == region.a_end)
            p = 0;
        else
            p = -1;

        for (long j = region.b_end; j >= 0; j--) {
            f = MAX(f, h - gapO) - gapE;
            EE[j] = MAX(EE[j], HH[j] - gapO) - gapE;

            h = p + SCORE_MATRIX_63(a_seq.seq[i], b_seq.seq[j]);

            if (f > h)
                h = f;
            if (EE[j] > h)
                h = EE[j];

            p = HH[j];

            HH[j] = h;

            if (h > cost) {
                cost = h;
                region.a_begin = i;
                region.b_begin = j;
                if (cost >= score) {
                    i = 0; // kind of a break here
                    j = 0;
//                    goto Found; TODO think about what to use here
                }
            }
        }
    }
//    ffatal("Internal error in align function.");
//    Found:

    free(EE);
    printf("3, a_seq.len: %ld\n", a_seq.len);
    free(HH);

    return region;
}

region_t init_region_for_global(sequence a_seq, sequence b_seq) {
    region_t region;
// TODO use it, to generalize the functions in cigar.c
    region.a_begin = 0;
    region.a_end = a_seq.len - 1;
    region.b_begin = 0;
    region.b_end = b_seq.len - 1;

    return region;
}

static void fill_alignment(alignment_p alignment, region_t region, cigar_p cigar) {
    alignment->align_q_start = region.a_begin;
    alignment->align_q_end = region.a_end;
    alignment->align_d_start = region.b_begin;
    alignment->align_d_end = region.b_end;
    alignment->alignment = cigar->cigar;
    alignment->alignment_len = cigar->len;
}

void align_nw_sellers(alignment_p alignment) {
    ffatal("align_nw_sellers: TODO not yet implemented");

    /*
     * TODO implement this
     */
}

void align_nw(alignment_p alignment) {
    sequence a_seq = { alignment->query.seq, alignment->query.len };
    sequence b_seq = { alignment->db_seq.seq, alignment->db_seq.len };

    region_t region = init_region_for_global(a_seq, b_seq);

    cigar_p cigar = compute_cigar_for_nw(a_seq, b_seq);

    fill_alignment(alignment, region, cigar);

    free_cigar(cigar);
}

void align_sw(alignment_p alignment) {
    sequence a_seq = { alignment->query.seq, alignment->query.len };
    sequence b_seq = { alignment->db_seq.seq, alignment->db_seq.len };

    region_t region = find_region_and_score_for_local(a_seq, b_seq);

    cigar_p cigar = compute_cigar_for_sw(a_seq, b_seq, region);

    fill_alignment(alignment, region, cigar);

    free_cigar(cigar);
}
