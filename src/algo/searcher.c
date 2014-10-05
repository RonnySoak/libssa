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
extern void it_free_sequence(p_sdb_sequence seq);

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
        long hit_count) {
    sdp = data;
    algo = algo_p;

    res = (p_search_result)xmalloc(sizeof(struct search_result));
    res->heap = minheap_init(hit_count);
    res->chunk_count = 0;
    res->seq_count = 0;
}

static void search_chunk(p_minheap heap, p_db_chunk chunk) {
    for (unsigned long i = 0; i < chunk->size; i++) {
        p_sdb_sequence dseq = chunk->seq[i];

        for (int x = 0; x < sdp->q_count; x++) {
            seq_buffer query = sdp->queries[x];

            elem_t * e = (elem_t *) xmalloc(sizeof(elem_t));
            e->query_id = x;
            e->db_id = dseq->info->ID;
            e->dframe = dseq->frame;
            e->dstrand = dseq->strand;

//            printf("search - i: %ld, DB ID: %ld\n", i, dseq->info->ID);
//
//            minheap_dump(heap);

            // TODO add other algorithms (7, 8, 16 bit)
            e->score = algo(&dseq->seq,
                        &query.seq,
                        sdp->hearray, // TODO how to cope with different parameters in different implementation?
                        score_matrix_63,
                        gapO, gapE);
//            printf("e.score: %ld\n", e->score);
            /*
             * Alignments, with a score equal to the current lowest score in the
             * heap are ignored!
             */
            minheap_add(heap, e);

            /*
             * minheap_add dereferences e and stores a copy of e, if its score
             * is higher than the lowest score in the heap.
             *
             * This means, we can and should free e here!
             */
            it_free_sequence((p_sdb_sequence) dseq);

            e->db_id = 0;
            e->dframe = 0;
            e->dstrand = 0;
            e->query_id = 0;
            e->score = 0;
            free(e);
            e = 0;
        }
    }
}

void s_free() {
    if (!res) {
        return;
    }

    for (int i = 0; i < res->heap->count; i++) {
        elem_t e = res->heap->array[i];

        e.db_id = 0;
        e.dframe = 0;
        e.dstrand = 0;
        e.query_id = 0;
        e.score = 0;
    }

    minheap_exit(res->heap);

    res->chunk_count = 0;
    res->seq_count = 0;
    free(res);
    res = 0;

    algo = 0;
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
