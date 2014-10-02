/*
 * manager.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../util/minheap.h"
#include "../internal_datatypes.h"

extern long fullsw(int8_t * dseq,
        int8_t * dend,
        int8_t * qseq,
        int8_t * qend,
        int64_t * hearray,
        int64_t * score_matrix,
        uint8_t gapopenextend,
        uint8_t gapextend);

extern void it_init(long chunk_count);

extern void s_init(p_search_data data,
        long (* algo_p) (int8_t *, int8_t *, int8_t *, int8_t *, int64_t *, int64_t *, uint8_t, uint8_t),
        long res_count);
extern p_result s_search();
extern void s_free(p_result p);

static p_search_data sdp;
unsigned long max_chunk_size = 3; // TODO change and/or make it configurable

void add_to_buffer(seq_buffer* buf, int8_t* seq, unsigned long len) {
    buf->qlen = len;
    buf->qtable = seq;
//                q.qtable = (uint8_t**) xmalloc(qlen * sizeof(uint8_t*));
//                for (int i = 0; i < qlen; i++) { TODO
//                    q.qtable[i] = sdp->dprofile + 64 * query->nt[s].seq[i];
//                }
}

p_search_data init_searchdata(p_query query) {
    p_search_data sdp = (p_search_data) xmalloc(sizeof(struct search_data));

    //    sdp->dbt = db_thread_create();
    sdp->dprofile = (uint8_t*) xmalloc(4 * 16 * 32);
    unsigned long qlen = 0;
    unsigned long hearraylen = 0;

    sdp->q_count = 0;

    if (symtype == NUCLEOTIDE) {
        for (int s = 0; s < 2; s++)
            if ((s + 1) & query_strands) {
                qlen = query->nt[s].len;

                add_to_buffer(&sdp->queries[sdp->q_count++], (int8_t*) query->nt[s].seq, qlen);

                hearraylen = qlen > hearraylen ? qlen : hearraylen;
            }
    }
    else if ((symtype == AMINOACID) || (symtype == TRANS_DB)) {
        qlen = query->aa[0].len;

        add_to_buffer(&sdp->queries[sdp->q_count++], (int8_t*) query->aa[0].seq, qlen);

        hearraylen = qlen > hearraylen ? qlen : hearraylen;
    }
    else if ((symtype == TRANS_QUERY) || (symtype == TRANS_BOTH)) {
        for (int s = 0; s < 2; s++)
            if ((s + 1) & query_strands)
                for (int f = 0; f < 3; f++) {
                    qlen = query->aa[3 * s + f].len;

                    add_to_buffer(&sdp->queries[sdp->q_count++], (int8_t*) query->aa[3 * s + f].seq, qlen);

                    hearraylen = qlen > hearraylen ? qlen : hearraylen;
                }
    }

    sdp->hearray = (uint8_t*) xmalloc(hearraylen * 32);

    // add information about number of strands and frames to search
    sdp->qstrand1 = 0;
    sdp->qframe1 = 0;
    sdp->qstrand2 = 0;
    sdp->qframe2 = 0;

    if (symtype == NUCLEOTIDE) {
        sdp->qstrand1 = (query_strands == COMPLEMENTARY_STRAND) ? 1 : 0;
        sdp->qstrand2 = (query_strands == FORWARD_STRAND) ? 0 : 1;
        // no translation, so no frames to search in
    }
    else if (symtype == TRANS_QUERY) {
        sdp->qstrand1 = (query_strands == COMPLEMENTARY_STRAND) ? 1 : 0;
        sdp->qstrand2 = (query_strands == FORWARD_STRAND) ? 0 : 1;
        sdp->qframe2 = 2;   // we search all three frames
    }
    else if (symtype == TRANS_BOTH) {
        sdp->qstrand1 = (query_strands == COMPLEMENTARY_STRAND) ? 1 : 0;
        sdp->qstrand2 = (query_strands == FORWARD_STRAND) ? 0 : 1;
        sdp->qframe2 = 2;   // we search all three frames
    }
    return sdp;
}

static void free_search_data() {
    if (!sdp) {
        return;
    }

    //    sdp->dbt = db_thread_create();
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

    sdp->qstrand1 = 0;
    sdp->qframe1 = 0;
    sdp->qstrand2 = 0;
    sdp->qframe2 = 0;

    free(sdp);
}

static void init(p_query query, int res_count) {
    sdp = init_searchdata(query);

    s_init(sdp, &fullsw, res_count);

    it_init(max_chunk_size);
}

void m_run(p_query query, int res_count) {
    // TODO add threads
    init(query, res_count);

    p_result res = s_search();
    minheap_sort(res->heap);

    // TODO start aligner

    s_free(res);
}

void m_exit() {
    free_search_data();
}
