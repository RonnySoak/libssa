/*
 * aligner.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../libssa_datatypes.h"

extern void align_sequences(char * a_seq,
        char * b_seq,
        unsigned long M,
        unsigned long N,
        long * scorematrix,
        long q,
        long r,
        long * a_begin,
        long * b_begin,
        long * a_end,
        long * b_end,
        char ** alignment,
        long * s);

extern p_seqinfo it_get_sequence(unsigned long id);
extern sequence it_translate_sequence(p_seqinfo info, int f, int s);

extern long * score_matrix_63;

extern uint8_t gapO;
extern uint8_t gapE;

static void init_alignment(p_alignment a, elem_t e, seq_buffer* queries) {
    p_seqinfo info = it_get_sequence(e.db_id);
    if (!info) {
        // TODO raise error, as this should not be possible
        ffatal("Could not get sequence from DB: %ld", e.db_id);
    }

    /*
     * TODO find a better way, maybe move code for translating based on symtype,
     * etc to util_sequence.c
     */
    sequence dseq = it_translate_sequence(info, e.dframe, e.dstrand);

    seq_buffer qseq = queries[e.query_id];

    a->db_seq.seq = dseq.seq;
    a->db_seq.len = dseq.len;
    a->db_seq.strand = e.dframe;
    a->db_seq.frame = e.dstrand;
    a->db_seq.header = info->header;
    a->db_seq.headerlen = info->headerlen;
    a->db_seq.ID = info->ID;

    a->query.seq = qseq.seq.seq;
    a->query.len = qseq.seq.len;
    a->query.strand = qseq.strand;
    a->query.frame = qseq.frame;

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
    alist->len = 0;

    free(alist);
}

p_alignment_list a_align(p_minheap heap, seq_buffer* queries, int q_count) {
    p_alignment_list alignment_list =
            (p_alignment_list) xmalloc(sizeof(struct alignment_list));
    alignment_list->alignments = (p_alignment*) xmalloc(heap->count * sizeof(struct alignment));
    alignment_list->len = heap->count;

    for (int i = 0; i < heap->count; i++) {
        // do alignment for each pair
        p_alignment a = (p_alignment) xmalloc(sizeof(struct alignment));
        init_alignment(a, heap->array[i], queries);

        align_sequences(a->query.seq,
                a->db_seq.seq,
                a->query.len,
                a->db_seq.len,
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
