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
#include <unistd.h>

#include "../util/minheap.h"
#include "../libssa_datatypes.h"
#include "../db_iterator.h"
#include "../query.h"
#include "align.h"
#include "aligner.h"
#include "searcher.h"
#include "search.h"

p_search_data sdp;
static void (*align_function)(alignment_p);

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

p_alignment_data prepare_alignment_data( int pair_count,
        elem_t * result_sequence_pairs) {
    p_alignment_data adp = xmalloc(sizeof(struct alignment_data));

    adp->align_function = align_function;

    adp->pair_count = pair_count;
    adp->result_sequence_pairs = result_sequence_pairs;

    adp->q_count = sdp->q_count;
    for (int i = 0; i < sdp->q_count; i++) {
        adp->queries[i] = sdp->queries[i];
    }

    return adp;
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

void free_alignment_data(p_alignment_data adp) {
    if (!adp) {
       return;
    }

    adp->q_count = 0;
    adp->align_function = 0;
    adp->result_sequence_pairs = 0;

    free(adp);
}

static void init(p_query query, int hit_count,
        int64_t (*search_algo)(sequence*, sequence*, int64_t*, int64_t*)) {
    init_searchdata(query, hit_count, search_algo);

    it_init(max_chunk_size);
}

void init_for_sw(p_query query, int hit_count) {
    init(query, hit_count, &full_sw);

    align_function = &align_sw;
}

void init_for_nw(p_query query, int hit_count) {
    init(query, hit_count, &full_nw);

    align_function = &align_nw;
}

void init_for_nw_sellers(p_query query, int hit_count) {
    init(query, hit_count, &full_nw_sellers);

    align_function = &align_nw_sellers;
}

/**
 * Run a search for query in the database. Aligns the query sequence against
 * each sequence in the DB and returns 'hit_count' alignments. The search is
 * configured through set bits in 'flags'.
 */
p_alignment_list m_run() {
    if (_max_thread_count == -1) {
        /*
         * TODO move this into a general config initializer
         */
        _max_thread_count = sysconf(_SC_NPROCESSORS_ONLN);
    }

    pthread_t thread_list[_max_thread_count];

    for (int i = 0; i < _max_thread_count; i++) {
        /*

    XXX synchronize it_get_next_chunk function (use mutex to read sequences)
        -> and prepare the sequences after reading them and releasing the mutex
         */
        pthread_create(&thread_list[i], NULL, s_search, sdp);
    }

    p_search_result search_result_list[_max_thread_count];
    p_minheap search_results = minheap_init(sdp->hit_count);

    for (int i = 0; i < _max_thread_count; i++) {
        pthread_join(thread_list[i], (void**) &search_result_list[i]);

        for (int j = 0; j < search_result_list[i]->heap->count; j++) {
            minheap_add(search_results, &search_result_list[i]->heap->array[j]);
        }
    }

    minheap_sort(search_results);
    it_free();

    /*
     * TODO write test and improve this
     */
    int alignments_per_thread = search_results->count / _max_thread_count;
    int rest = search_results->count % _max_thread_count;

    int align_thread_count;
    if (alignments_per_thread == 0) {
        align_thread_count = rest;
    }
    else {
        align_thread_count = _max_thread_count;
    }

    printf("alignments_per_thread: %d\n", alignments_per_thread);

    p_alignment_data adp_list[_max_thread_count];
    int search_result_ptr = 0;
    for (int i = 0; i < align_thread_count; i++) {
        printf("search_result_ptr: %d\n", search_result_ptr);

        adp_list[i] = prepare_alignment_data(alignments_per_thread,
                search_results->array + search_result_ptr);

//        if (i < rest) {
//            adp_list[i]->
//        }

        pthread_create(&thread_list[i], NULL, a_align, adp_list[i]);

        search_result_ptr += alignments_per_thread;
    }

    p_alignment_list alist = xmalloc(sdp->hit_count * sizeof(struct alignment_list));
    alist->alignments = xmalloc(sdp->hit_count * sizeof(alignment_t));
    alist->len = sdp->hit_count;

    p_alignment_list align_result_list[_max_thread_count];

    int alist_ptr = 0;
    for (int i = 0; i < align_thread_count; i++) {
        pthread_join(thread_list[i], (void**) &align_result_list[i]);

        printf("a count: %d\n", align_result_list[i]->len);

        for(int j = 0; j < align_result_list[i]->len; j++) {
            printf("alist_ptr: %d\n", alist_ptr);
            alist->alignments[alist_ptr++] = align_result_list[i]->alignments[j];
        }
    }

    minheap_exit(search_results);

    free_search_data();
    for (int i = 0; i < _max_thread_count; i++) {
        s_free(search_result_list[i]);
    }
    for (int i = 0; i < align_thread_count; i++) {
        free_alignment_data(adp_list[i]);
    }

    return alist;
}
