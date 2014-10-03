/*
 * aligner.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../libssa_datatypes.h"

extern void align_sequences(sequence * a_seq,
        sequence * b_seq,
        long * scorematrix,
        long q,
        long r,
        long * a_begin,
        long * b_begin,
        long * a_end,
        long * b_end,
        char ** alignment,
        long * s);

extern long * score_matrix_63;

extern uint8_t gapO;
extern uint8_t gapE;

static void init_alignment(p_alignment a, p_sdb_sequence dseq, seq_buffer* qseq) {
    a->db_seq.seq = dseq->seq.seq;
    a->db_seq.len = dseq->seq.len;
    a->db_seq.strand = dseq->strand;
    a->db_seq.frame = dseq->frame;
    a->db_seq.header = dseq->info->header;
    a->db_seq.headerlen = dseq->info->headerlen;
    a->db_seq.ID = dseq->info->ID;

    a->query.seq = qseq->seq.seq;
    a->query.len = qseq->seq.len;
    a->query.strand = qseq->strand;
    a->query.frame = qseq->frame;

    a->align_q_start = 0;
    a->align_d_start = 0;
    a->align_q_end = 0;
    a->align_d_end = 0;
    a->alignment = 0;
    a->score = 0;
    a->score_align = 0;
}

void a_free(p_alignment_list alist) {
    if (!alist) {
        return;
    }

    if (alist->alignments) {
        for (int i = 0; i < alist->len; i++) {
            if (alist->alignments[i]) {
                p_alignment a = alist->alignments[i];

                a->db_seq.seq = 0;
                a->db_seq.len = 0;
                a->db_seq.strand = 0;
                a->db_seq.frame = 0;
                a->db_seq.header = 0;
                a->db_seq.headerlen = 0;
                a->db_seq.ID = 0;

                a->query.seq = 0;
                a->query.len = 0;
                a->query.strand = 0;
                a->query.frame = 0;

                a->align_q_start = 0;
                a->align_d_start = 0;
                a->align_q_end = 0;
                a->align_d_end = 0;
                if (a->alignment) {
                    free(a->alignment);
                    a->alignment = 0;
                }
                a->score = 0;
                a->score_align = 0;

                free(a);
            }
            alist->alignments[i] = 0;
        }
        free(alist->alignments);
    }
    alist->alignments = 0;
    free(alist);
}

p_alignment_list a_align(p_minheap heap) {
    p_alignment_list alignment_list =
            (p_alignment_list) xmalloc(sizeof(struct alignment_list));
    alignment_list->alignments = (p_alignment*) xmalloc(heap->count * sizeof(struct alignment));
    alignment_list->len = heap->count;

    for (int i = 0; i < heap->count; i++) {
        // do alignment for each pair
        p_sdb_sequence db_seq = (p_sdb_sequence)heap->array[i].db_seq;
        seq_buffer * qseq = (seq_buffer*)heap->array[i].query;

        p_alignment a = (p_alignment) xmalloc(sizeof(struct alignment));

        init_alignment(a, db_seq, qseq);

        align_sequences(&qseq->seq,
                &db_seq->seq,
                score_matrix_63,
                gapO,
                gapE,
                & a->align_q_start,
                & a->align_d_start,
                & a->align_q_end,
                & a->align_d_end,
                & a->alignment,
                & a->score_align);

        alignment_list->alignments[i] = a;
    }

    return alignment_list;
}
