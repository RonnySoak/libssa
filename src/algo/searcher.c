/*
 * searcher.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "search.h"

#include <stdlib.h>
#include <assert.h>

#include "../util.h"
#include "../util/minheap.h"
#include "../db_iterator.h"
#include "../matrices.h"

static unsigned long search_chunk(p_minheap heap, p_db_chunk chunk, p_search_data sdp,
        int64_t* hearray) {
	unsigned long searches_done = 0;

    for (unsigned long i = 0; i < chunk->size; i++) {
        p_sdb_sequence dseq = chunk->seq[i];

        if (!dseq->seq.len) {
        	printf("length = 0\n");
        }

        for (int x = 0; x < sdp->q_count; x++) {
            seq_buffer query = sdp->queries[x];

            elem_t * e = xmalloc(sizeof(elem_t));
            e->query_id = x;
            e->db_id = dseq->ID;
            e->dframe = dseq->frame;
            e->dstrand = dseq->strand;

            e->score = sdp->search_algo(&dseq->seq,
                        &query.seq,
                        hearray, // TODO how to cope with different parameters in different implementation?
                        score_matrix_63);

            searches_done++;

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
            e->db_id = 0;
            e->dframe = 0;
            e->dstrand = 0;
            e->query_id = 0;
            e->score = 0;
            free(e);
            e = 0;
        }
    }

    return searches_done;
}

void s_free(p_search_result res) {
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
}

void * s_search(void * search_data) {
    p_search_data sdp = (p_search_data) search_data;

    p_search_result res = xmalloc(sizeof(struct search_result));
    res->heap = minheap_init(sdp->hit_count);
    res->chunk_count = 0;
    res->seq_count = 0;

    int64_t* hearray = xmalloc(sdp->hearraylen * 32);

    p_db_chunk chunk;
    while((chunk = it_next_chunk())) {
        int searched_sequences = search_chunk(res->heap, chunk, sdp, hearray);

        assert(searched_sequences == chunk->size * sdp->q_count);

        res->chunk_count++;
        res->seq_count += chunk->size;

        it_free_chunk(chunk);
    }

    free(hearray);

    return res;
}
