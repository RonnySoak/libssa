/*
 * aligner.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "../libssa_datatypes.h"
#include "searcher.h"

#include "../util.h"

/*
 * TODO switch to SWARM/vsearch implementation of this
 *
 * Have a look at cigar string and use that
 *
 * Compare with the cigar of SSW
 *
 * TODO reduce number of malloc calls to speed it up
 */

#define MAX(a,b) (a > b ? a : b)

void pushop(char newop, char ** cigarendp, char * op, int * count) {
    if (newop == *op)
        (*count)++;
    else {
        *--*cigarendp = *op;
        if (*count > 1) {
            char buf[25];
            int len = sprintf(buf, "%d", *count);
            *cigarendp -= len;
            strncpy(*cigarendp, buf, len);
        }
        *op = newop;
        *count = 1;
    }
}

void finishop(char ** cigarendp, char * op, int * count) {
    if ((op) && (count)) {
        *--*cigarendp = *op;
        if (*count > 1) {
            char buf[25];
            int len = sprintf(buf, "%d", *count);
            *cigarendp -= len;
            strncpy(*cigarendp, buf, len);
        }
        *op = 0;
        *count = 0;
    }
}

void align(sequence * qseq, sequence * dseq, long * score_matrix,
        unsigned char * directions) {
    /* backtrack: count differences and save alignment in cigar string */
//
//    long score = 0;
//    long alength = 0;
//    long matches = 0;
//
//    char * cigar = xmalloc(qseq->len + dseq->len + 1);
//    char * cigarend = cigar + qseq->len + dseq->len + 1;
//
//    char op = 0;
//    int count = 0;
//    *(--cigarend) = 0;
//
//    unsigned long i = qseq->len;
//    unsigned long j = dseq->len;
//
//    while ((i > 0) && (j > 0)) {
//        int d = directions[qseq->len * (j - 1) + (i - 1)];
//
//        alength++;
//
//        if ((op == 'I') && (d & maskextleft)) {
//            score += gapE;
//            j--;
//            pushop('I', &cigarend, &op, &count);
//        }
//        else if ((op == 'D') && (d & maskextup)) {
//            score += gapE;
//            i--;
//            pushop('D', &cigarend, &op, &count);
//        }
//        else if (d & maskleft) {
//            score += gapE;
//            if (op != 'I')
//                score += gapO;
//            j--;
//            pushop('I', &cigarend, &op, &count);
//        }
//        else if (d & maskup) {
//            score += gapE;
//            if (op != 'D')
//                score += gapO;
//            i--;
//            pushop('D', &cigarend, &op, &count);
//        }
//        else {
//            score += score_matrix[(dseq[j - 1] << 5) + qseq[i - 1]];
//            if (qseq[i - 1] == dseq[j - 1])
//                matches++;
//            i--;
//            j--;
//            pushop('M', &cigarend, &op, &count);
//        }
//    }
//
//    while (i > 0) {
//        alength++;
//        score += gapE;
//        if (op != 'D')
//            score += gapO;
//        i--;
//        pushop('D', &cigarend, &op, &count);
//    }
//
//    while (j > 0) {
//        alength++;
//        score += gapE;
//        if (op != 'I')
//            score += gapO;
//        j--;
//        pushop('I', &cigarend, &op, &count);
//    }
//
//    finishop(&cigarend, &op, &count);
//
//    /* move and reallocate cigar */
//
//    long cigarlength = cigar + qseq->len + dseq->len - cigarend;
//    memmove(cigar, cigarend, cigarlength + 1);
//    cigar = xrealloc(cigar, cigarlength + 1);
//
//    *nwscore = dist;
//    *nwdiff = alength - matches;
//    *nwalignmentlength = alength;
//    *nwalignment = cigar;
//
//    if (score != dist) {
//        fprintf(stderr, "Error with query no %lu and db sequence no %lu:\n",
//                queryno, dbseqno);
//        fprintf(stderr,
//                "Initial and recomputed alignment score disagreement: %lu %lu\n",
//                dist, score);
//        fprintf(stderr, "Alignment: %s\n", cigar);
//    }
}

void align_sequences(char * a_seq, char * b_seq, unsigned long M,
        unsigned long N, long * scorematrix, long q, long r, long * a_begin,
        long * b_begin, long * a_end, long * b_end, char ** alignment, long * s) {
    struct aligner_info ai;

    long score = *s;

    printf("a: %s, b: %s, alen: %ld, blen: %ld\n", a_seq, b_seq, M, N);

    init(&ai);
    region(a_seq, b_seq, M, N, scorematrix, q, r, a_begin, b_begin, a_end,
            b_end, &score);

    diff(&ai, a_seq, b_seq, *a_end - *a_begin + 1, *b_end - *b_begin + 1,
            *a_begin, *b_begin, scorematrix, q, r, q, q);

    push(&ai);

    *alignment = ai.alignment;
    *s = score;
}
