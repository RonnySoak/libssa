/*
 * manager.c
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "manager.h"

#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include <assert.h>

#include "../util/minheap.h"
#include "../util/thread_pool.h"
#include "../libssa_datatypes.h"
#include "../db_iterator.h"
#include "../query.h"
#include "align.h"
#include "aligner.h"
#include "searcher.h"
#include "search.h"

p_search_data sdp;
p_alignment_data adp;

static unsigned long max_chunk_size = 1000; // TODO change and/or make it configurable

static void add_to_buffer(seq_buffer* buf, sequence seq, int strand,
        int frame) {
    buf->seq = seq;
    buf->frame = frame;
    buf->strand = strand;
//                q.qtable = xmalloc(qlen * sizeof(uint8_t*));
//                for (int i = 0; i < qlen; i++) { TODO
//                    q.qtable[i] = sdp->dprofile + 64 * query->nt[s].seq[i];
//                }
}

static void init_alignment_data(void (*align_function)(alignment_p)) {
    adp = xmalloc(sizeof(struct alignment_data));

    adp->align_function = align_function;

    adp->q_count = sdp->q_count;
    for (int i = 0; i < sdp->q_count; i++) {
        adp->queries[i] = sdp->queries[i];
    }
}

static void init_searchdata(p_query query, int hit_count,
        int64_t (*search_algo)(sequence*, sequence*, int64_t*, int64_t*)) {
    sdp = xmalloc(sizeof(struct search_data));

    sdp->hit_count = hit_count;
    sdp->search_algo = search_algo;

    sdp->dprofile = xmalloc(4 * 16 * 32);
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

    sdp->hearraylen = hearraylen;
}

void free_search_data() {
    if (!sdp) {
        return;
    }

    if (sdp->dprofile) {
        free(sdp->dprofile);
        sdp->dprofile = 0;
    }

    sdp->q_count = 0;
    sdp->hit_count = 0;
    sdp->search_algo = 0;

    free(sdp);
}

void free_alignment_data() {
    if (!adp) {
       return;
    }

    adp->q_count = 0;
    adp->align_function = 0;
    adp->result_sequence_pairs = 0;

    free(adp);
}

static void init(p_query query, int hit_count,
        int64_t (*search_algo)(sequence*, sequence*, int64_t*, int64_t*),
        void (*align_function)(alignment_p)) {
    init_searchdata(query, hit_count, search_algo);

    init_alignment_data(align_function);

    it_init(max_chunk_size);
}

void init_for_sw(p_query query, int hit_count) {
    init(query, hit_count, &full_sw,  &align_sw);
}

void init_for_nw(p_query query, int hit_count) {
    init(query, hit_count, &full_nw, &align_nw);
}

void init_for_nw_sellers(p_query query, int hit_count) {
    init(query, hit_count, &full_nw_sellers, &align_nw_sellers);
}

/**
 * Run a search for query in the database. Aligns the query sequence against
 * each sequence in the DB and returns 'hit_count' alignments. The search is
 * configured through set bits in 'flags'.
 */
p_alignment_list m_run() {
    init_thread_pool();

    start_threads_unified_arguments(s_search, sdp);

    p_search_result search_result_list[_max_thread_count];

    unsigned long chunks_processed = 0;
    unsigned long db_sequences_processed = 0;

    wait_for_threads((void **)&search_result_list);
    p_minheap search_results = minheap_init(sdp->hit_count);
    for (int i = 0; i < get_current_thread_count(); i++) {
        for (int j = 0; j < search_result_list[i]->heap->count; j++) {
            minheap_add(search_results, &search_result_list[i]->heap->array[j]);
        }

        chunks_processed += search_result_list[i]->chunk_count;
        db_sequences_processed += search_result_list[i]->seq_count;
    }

    printf("db sequence count: %ld\n", ssa_db_get_sequence_count());
    printf("db sequences processed: %ld\n", db_sequences_processed);
    printf("chunks expected: %lf\n", ceil(ssa_db_get_sequence_count() / (double)max_chunk_size));
    printf("chunks processed: %ld\n", chunks_processed);

    assert(ssa_db_get_sequence_count() == db_sequences_processed);
    assert(ceil(ssa_db_get_sequence_count() / (double) max_chunk_size) == chunks_processed);

    minheap_sort(search_results);
    it_free();

    adp->pair_count = search_results->count;
    adp->result_sequence_pairs = search_results->array;
    set_alignment_data(adp);

    start_threads_unified_arguments(a_align, NULL);

    p_alignment_list alist = xmalloc(sdp->hit_count * sizeof(struct alignment_list));
    alist->alignments = xmalloc(sdp->hit_count * sizeof(alignment_t));
    alist->len = sdp->hit_count;

    p_alignment_list align_result_list[get_current_thread_count()];

    wait_for_threads((void **)&align_result_list);

    int alist_ptr = 0;
    for (int i = 0; i < get_current_thread_count(); i++) {
        for(int j = 0; j < align_result_list[i]->len; j++) {
            alist->alignments[alist_ptr++] = align_result_list[i]->alignments[j];
        }

        free(align_result_list[i]->alignments);
        free(align_result_list[i]);
    }

    minheap_exit(search_results);

    free_search_data();
    free_alignment_data();
    for (int i = 0; i < _max_thread_count; i++) {
        s_free(search_result_list[i]);
    }

    return alist;
}
