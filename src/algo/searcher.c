/*
 * searcher.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "search.h"

#include <stdlib.h>
#include <assert.h>

#include "../util/util.h"
#include "../util/minheap.h"
#include "../db_iterator.h"
#include "../matrices.h"

static unsigned long search_chunk(p_minheap heap, p_db_chunk chunk, p_search_data sdp,
        int64_t* hearray) {
	unsigned long searches_done = 0;

    for (unsigned long i = 0; i < chunk->fill_pointer; i++) {
        p_sdb_sequence dseq = chunk->seq[i];

        for (int q_id = 0; q_id < sdp->q_count; q_id++) {
            seq_buffer query = sdp->queries[q_id];

            long score = sdp->search_algo(&dseq->seq,
                    &query.seq,
                    hearray, // TODO how to cope with different parameters in different implementations?
                    score_matrix_63);

            add_to_minheap( heap, q_id, dseq, score );

            searches_done++;
        }

//        it_free_sequence( dseq );
    }

    return searches_done;
}

void s_free(p_search_result res) {
    if (!res) {
        return;
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

    p_db_chunk chunk = it_new_chunk();
    it_next_chunk(chunk);

    while(chunk->fill_pointer) {
        int searched_sequences = search_chunk(res->heap, chunk, sdp, hearray);

        assert(searched_sequences == chunk->fill_pointer * sdp->q_count);

        res->chunk_count++;
        res->seq_count += chunk->fill_pointer;

        it_next_chunk(chunk);
    }

	it_free_chunk(chunk);

    free(hearray);

    return res;
}
