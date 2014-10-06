/*
 * manager.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../util/minheap.h"
#include "../libssa_datatypes.h"

extern long fullsw(sequence * dseq,
        sequence * qseq,
        int64_t * hearray,
        int64_t * score_matrix,
        uint8_t gapopenextend,
        uint8_t gapextend);

extern void it_init(long chunk_count);
extern void it_free();

extern void s_init(p_search_data data,
        long (* algo_p) (sequence *, sequence *, int64_t *, int64_t *, uint8_t, uint8_t),
        long hit_count);
extern p_search_result s_search();
extern void s_free(p_search_result p);

extern p_alignment_list a_align(p_minheap heap, seq_buffer* queries, int q_count);

static p_search_data sdp;
unsigned long max_chunk_size = 100; // TODO change and/or make it configurable

void add_to_buffer(seq_buffer* buf, sequence seq, int strand,
        int frame) {
    buf->seq = seq;
    buf->frame = frame;
    buf->strand = strand;
//                q.qtable = (uint8_t**) xmalloc(qlen * sizeof(uint8_t*));
//                for (int i = 0; i < qlen; i++) { TODO
//                    q.qtable[i] = sdp->dprofile + 64 * query->nt[s].seq[i];
//                }
}

p_search_data init_searchdata(p_query query) {
    p_search_data sdp = (p_search_data) xmalloc(sizeof(struct search_data));

    // TODO   sdp->dbt = db_thread_create();
    sdp->dprofile = (uint8_t*) xmalloc(4 * 16 * 32);
    unsigned long qlen = 0;
    unsigned long hearraylen = 0;

    sdp->q_count = 0;

    // init buffer
    for (int i = 0; i < 6; i++) {
        sdp->queries[i].seq.len = 0;
        sdp->queries[i].seq.seq = 0;
        sdp->queries[i].strand = 0;
        sdp->queries[i].frame = 0;
    }

    if (symtype == NUCLEOTIDE) {
        for (int s = 0; s < 2; s++)
            if ((s + 1) & query_strands) {
                qlen = query->nt[s].len;

                add_to_buffer(&sdp->queries[sdp->q_count++], query->nt[s], s, 0);

                hearraylen = qlen > hearraylen ? qlen : hearraylen;
            }
    }
    else if ((symtype == AMINOACID) || (symtype == TRANS_DB)) {
        qlen = query->aa[0].len;

        add_to_buffer(&sdp->queries[sdp->q_count++], query->aa[0], 0, 0);

        hearraylen = qlen > hearraylen ? qlen : hearraylen;
    }
    else if ((symtype == TRANS_QUERY) || (symtype == TRANS_BOTH)) {
        for (int s = 0; s < 2; s++)
            if ((s + 1) & query_strands)
                for (int f = 0; f < 3; f++) {
                    qlen = query->aa[3 * s + f].len;

                    add_to_buffer(&sdp->queries[sdp->q_count++],
                            query->aa[3 * s + f], s, f);

                    hearraylen = qlen > hearraylen ? qlen : hearraylen;
                }
    }

    sdp->hearray = (uint8_t*) xmalloc(hearraylen * 32);

    return sdp;
}

void free_search_data() {
    if (!sdp) {
        return;
    }

    //  TODO  sdp->dbt = db_thread_create();
    if (sdp->dprofile) {
        free(sdp->dprofile);
        sdp->dprofile = 0;
    }

    // TODO free data for qtable in seq_buffer
    sdp->q_count = 0;

    if (sdp->hearray) {
        free(sdp->hearray);
        sdp->hearray = 0;
    }

    free(sdp);
}

static void init(p_query query, int hit_count) {
    sdp = init_searchdata(query);

    s_init(sdp, &fullsw, hit_count);

    it_init(max_chunk_size);
}

p_alignment_list m_run(p_query query, int hit_count) {
    // TODO add threads
    init(query, hit_count);

    p_search_result res = s_search();

    it_free();

    minheap_sort(res->heap);

    p_alignment_list alist = a_align(res->heap, sdp->queries, sdp->q_count);

    free_search_data();
    s_free(res);

    return alist;
}
