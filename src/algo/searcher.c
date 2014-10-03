/*
 * searcher.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../util.h"
#include "../util/minheap.h"
#include "../libssa_datatypes.h"

extern p_db_chunk it_next_chunk();
extern void it_free_chunk(p_db_chunk chunk);

extern long * score_matrix_63;

static p_search_data sdp;
static long (* algo) (sequence *, sequence *, int64_t *, int64_t *, uint8_t, uint8_t);
        // TODO make pointer from sequences!!!
static p_search_result res = 0;

// both are filled from libssa.c
uint8_t gapO;
uint8_t gapE;

void s_init(p_search_data data,
        long (* algo_p) (sequence *, sequence *, int64_t *, int64_t *, uint8_t, uint8_t),
        long res_count) {
    sdp = data;
    algo = algo_p;

    res = (p_search_result)xmalloc(sizeof(struct search_result));
    res->heap = minheap_init(res_count);
    res->chunk_count = 0;
    res->seq_count = 0;
}

static void search_chunk(p_minheap heap, p_db_chunk chunk) {
    for (int i = 0; i < chunk->size; i++) {
        p_sdb_sequence dseq = chunk->seq[i];

        for (int x = 0; x < sdp->q_count; x++) {
            seq_buffer query = sdp->queries[x];

            elem_t* e = (elem_t*)xmalloc(sizeof(elem_t));
            e->query = &query;
            e->db_seq = dseq;

            // TODO add other algorithms (7, 8, 16 bit)
            e->score = algo(&dseq->seq,
                        &query.seq,
                        sdp->hearray, // TODO how to cope with different parameters in different implementation?
                        score_matrix_63,
                        gapO, gapE);

            // TODO get old element/sequence as return value and free it!!
            minheap_add(heap, e);
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

p_search_result s_search() {
    p_db_chunk chunk;
    while((chunk = it_next_chunk())) {
        search_chunk(res->heap, chunk);

        res->chunk_count++;
        res->seq_count += chunk->size;

        it_free_chunk(chunk);
    }

    return res;
}
