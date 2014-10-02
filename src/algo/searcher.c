/*
 * searcher.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../util.h"
#include "../util/minheap.h"
#include "../internal_datatypes.h"

extern p_db_chunk it_next_chunk();
extern void it_free_chunk(p_db_chunk chunk);

extern long * score_matrix_63;

static p_search_data sdp;
static long (* algo) (int8_t *, int8_t *, int8_t *, int8_t *, int64_t *, int64_t *, uint8_t, uint8_t);
static p_result res = 0;

// both are filled from libssa.c
uint8_t gapO;
uint8_t gapE;

void s_init(p_search_data data,
        long (* algo_p) (int8_t *, int8_t *, int8_t *, int8_t *, int64_t *, int64_t *, uint8_t, uint8_t),
        long res_count) {
    sdp = data;
    algo = algo_p;

    res = (p_result)xmalloc(sizeof(struct result_t));
    res->heap = minheap_init(res_count);
    res->chunk_count = 0;
    res->seq_count = 0;
}

static void search_chunk(p_minheap heap, p_db_chunk chunk) {
    for (int i = 0; i < chunk->size; i++) {
        p_sdb_sequence dseq = chunk->seq[i];

        for (int x = 0; x < sdp->q_count; x++) {
            seq_buffer query = sdp->queries[x];

            elem_t e;
            e.query = &query;
            e.db_seq = dseq;

            // TODO add other algorithms (7, 8, 16 bit)
            e.score = algo(dseq->seq,
                        dseq->seq + dseq->len,
                        query.qtable, query.qtable + query.qlen,
                        sdp->hearray, // TODO how to cope with different parameters in different implementation?
                        score_matrix_63,
                        gapO, gapE);

            minheap_add(heap, &e);
        }
    }
}

void s_free() {
    if (!res) {
        return;
    }
    minheap_exit(res->heap);
    free(res);
}

p_result s_search() {
    p_db_chunk chunk;
    while((chunk = it_next_chunk())) {
        search_chunk(res->heap, chunk);

        res->chunk_count++;
        res->seq_count += chunk->size;

        it_free_chunk(chunk);
    }

    return res;
}
