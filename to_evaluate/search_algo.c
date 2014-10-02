/*
 * search_algo.c
 *
 *  Created on: Sep 21, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "util.h"
#include "minheap.h"

// from db_iterator.c
extern p_sdb_sequence it_next();
extern void it_init();


static p_alignment_list init_alignment_list(long length) {
    p_alignment_list result = (p_alignment_list) xmalloc(
            sizeof(struct alignment_list));

    for (int i = 0; i < length; i++) {
        // TODO how do I get a contiguous memory space

        // init each alignment
    }

    return result;
}

void free_alignment_list(p_alignment_list list) {
    if (!list) {
        return;
    }

    for (int i = 0; i < list->length; i++) {
        if (list->alignments[i]) {
            // TODO free alignment
        }

        free(list->alignments[i]);
    }

    list->alignments = 0;
    list->length = 0;
    free(list);
}

static long do_search(p_search_data sdp, p_query query, p_sdb_sequence dseq) {
    //  fprintf(out, "Searching seqnos %ld to %ld\n", sdp->seqfirst, sdp->seqlast);

//        if (taxidfilename)
//            db_mapheaders(sdp->dbt, sdp->seqfirst, sdp->seqlast);

//        sdp->start_count = 0;             TODO for chunk handling
//        for (long seqno = sdp->seqfirst; seqno <= sdp->seqlast; seqno++) {
//            if (db_check_inclusion(sdp->dbt, seqno)) {
//                if ((symtype == 3) || (symtype == 4)) {
//                    for (long dstrand = sdp->dstrand1; dstrand <= sdp->dstrand2;
//                            dstrand++)
//                        for (long dframe = sdp->dframe1; dframe <= sdp->dframe2;
//                                dframe++) {
//                            sdp->start_list[sdp->start_count++] = (seqno << 3)
//                                    | (dstrand << 2) | dframe;
//                        }
//                }
//                else {
//                    sdp->start_list[sdp->start_count++] = seqno << 3;
//                }
//            }
//        }
//
//        if (sdp->start_count == 0)
//            return;
//
//        long s1 = sdp->start_list[0] >> 3;
//        long s2 = sdp->start_list[sdp->start_count - 1] >> 3;

//        // fprintf(out, "Mapping seqnos %ld to %ld\n", s1, s2);
//        db_mapsequences(sdp->dbt, s1, s2);

        const long dstrand = dseq->strand;
        const long dframe = dseq->frame;

        for (int i = 0; i < sdp->q_count; i++) {

//        for (long qstrand = sdp->qstrand1; qstrand <= sdp->qstrand2; qstrand++)
//            for (long qframe = sdp->qframe1; qframe <= sdp->qframe2; qframe++) {

                sdp->out_count = sdp->start_count;
                memcpy(sdp->out_list, sdp->start_list,
                        sdp->start_count * sizeof(long));

                seq_buffer q_buf = sdp->queries[i];

                BYTE ** qtable = q_buf->qtable;
                long qlen = q_buf->qlen;

//    #if 1
//
//                /* 7-bit search */
//
//                sdp->tmp_list = sdp->in_list;
//                sdp->in_list = sdp->out_list;
//                sdp->out_list = sdp->tmp_list;
//                sdp->in_count = sdp->out_count;
//
//                if (sdp->in_count > 0) {
////                    pthread_mutex_lock(&countmutex);
////                    compute7 += sdp->in_count;
////                    rounds7++;
////                    pthread_mutex_unlock(&countmutex);
//
//                    // fprintf(out, "Searching seqnos %ld to %ld\n", sdp->in_list[0], sdp->in_list[sdp->in_count-1]);
//
//                    if (cpu_feature_ssse3)
//                        search7_ssse3(qtable, gapopenextend, gapextend,
//                                (BYTE*) score_matrix_7t, sdp->dprofile,
//                                sdp->hearray, sdp->dbt, sdp->in_count, sdp->in_list,
//                                sdp->scores, qlen);
//                    else
//                        search7(qtable, gapopenextend, gapextend,
//                                (BYTE*) score_matrix_7, sdp->dprofile, sdp->hearray,
//                                sdp->dbt, sdp->in_count, sdp->in_list, sdp->scores,
//                                qlen);
//
//                    sdp->out_count = 0;
//
//                    for (int i = 0; i < sdp->in_count; i++) {
//                        long seqnosf = sdp->in_list[i];
//                        long score = sdp->scores[i];
//
//                        if (score < SCORELIMIT_7) {
//                            long seqno = seqnosf >> 3;
//
//                            if ((symtype == 0) && q_buf->strand)
//                                hits_enter(score, 0, 0, dseq, -1, -1);
//                            else
//                                hits_enter(score, q_buf->strand, q_buf->frame, dseq, -1, -1);
//                        }
//                        else {
//                            sdp->out_list[sdp->out_count++] = seqnosf;
//                        }
//                    }
//                }
//    #endif
//
//    #if 1
//
//                /* 16-bit search */
//
//                sdp->tmp_list = sdp->in_list;
//                sdp->in_list = sdp->out_list;
//                sdp->out_list = sdp->tmp_list;
//                sdp->in_count = sdp->out_count;
//
//                if (sdp->in_count > 0) {
////                    pthread_mutex_lock(&countmutex);
////                    compute16 += sdp->in_count;
////                    rounds16++;
////                    pthread_mutex_unlock(&countmutex);
//
//                    search16((WORD**) qtable, gapopenextend, gapextend,
//                            (WORD*) (score_matrix_16), (WORD*) (sdp->dprofile),
//                            (WORD*) (sdp->hearray), sdp->dbt, sdp->in_count,
//                            sdp->in_list, sdp->scores, sdp->bestpos, qlen);
//
//                    sdp->out_count = 0;
//
//                    for (int i = 0; i < sdp->in_count; i++) {
//                        long seqnosf = sdp->in_list[i];
//                        long score = sdp->scores[i];
//                        if (score < SCORELIMIT_16) {
//                            long seqno = seqnosf >> 3;
//
//                            long pos = sdp->bestpos[i];
//
//                            //      fprintf(out, "seqno=%ld score=%ld bestpos=%ld\n", seqno, score, pos);
//
//                            if ((symtype == 0) && q_buf->strand)
//                                hits_enter(score, 0, 0, dseq, pos, -1);
//                            else
//                                hits_enter(score, q_buf->strand, q_buf->frame, dseq, pos, -1);
//                        }
//                        else {
//                            sdp->out_list[sdp->out_count++] = seqnosf;
//                        }
//                    }
//                }
//    #endif

    #if 1

                /* 63-bit search */

//                sdp->tmp_list = sdp->in_list; TODO for chunk handling
//                sdp->in_list = sdp->out_list;
//                sdp->out_list = sdp->tmp_list;
//                sdp->in_count = sdp->out_count;
//
//                if (sdp->in_count > 0) {
////                    pthread_mutex_lock(&countmutex);
////                    compute63 += sdp->in_count;
////                    rounds63++;
////                    pthread_mutex_unlock(&countmutex);
//
//                    for (int i = 0; i < sdp->in_count; i++) {
//                        long seqnosf = sdp->in_list[i];

                        char * dbegin = dseq->seq;
                        char * dend = dbegin + dseq->len - 1;

                        char * q;
                        if (symtype == 0)
                            q = query.nt[q_buf->strand].seq;
                        else
                            q = query.aa[3 * q_buf->strand + q_buf->frame].seq;

                        long score = fullsw(
                                dbegin, dend,                   // DB
                                (char*) q, (char*) q + qlen,    // query
                                (long*) sdp->hearray,
                                score_matrix_63,
                                gapopenextend,
                                gapextend);

                        if ((symtype == NUCLEOTIDE) && q_buf->strand)
                            hits_enter(score, 0, 0, dseq, -1, -1);
                        else
                            hits_enter(score, q_buf->strand, q_buf->frame, dseq, -1, -1);
//                    }
//                }

    #endif
            }
//    return fullsw(dseq->seq, dseq->seq + dseq->len,
//            qseq, qseq + qlen,
//            hearray,
//            score_matrix_63,
//            gapO, gapE);
}

void search_init(p_query query, p_search_data sdp) {
    sdp->dbt = db_thread_create();
    sdp->dprofile = (uint8_t*) xmalloc(4 * 16 * 32);
    long qlen = 0;
    long hearraylen = 0;

    for (int i = 0; i < 6; i++) {
        sdp->queries[i] = 0;
    }

    if (symtype == NUCLEOTIDE) {
        for (int s = 0; s < 2; s++)
            if ((s + 1) & query_strands) {
                qlen = query.nt[s].len;

                seq_buffer * q = sdp->queries[3 * s];

                q->qlen = qlen;
                q->qtable = (uint8_t**) xmalloc(qlen * sizeof(uint8_t*));
                for (int i = 0; i < qlen; i++) {
                    q->qtable[i] = sdp->dprofile + 64 * query.nt[s].seq[i];
                }
                hearraylen = qlen > hearraylen ? qlen : hearraylen;
            }
    }
    else if ((symtype == AMINOACID) || (symtype == TRANS_DB)) {
        qlen = query.aa[0].len;

        seq_buffer * q = sdp->queries[0];

        q->qlen = qlen;
        q->qtable = (uint8_t**) xmalloc(qlen * sizeof(uint8_t*));
        for (int i = 0; i < qlen; i++) {
            q->qtable[i] = sdp->dprofile + 64 * query.aa[0].seq[i];
        }
        hearraylen = qlen > hearraylen ? qlen : hearraylen;
    }
    else if ((symtype == TRANS_QUERY) || (symtype == TRANS_BOTH)) {
        for (int s = 0; s < 2; s++)
            if ((s + 1) & query_strands)
                for (int f = 0; f < 3; f++) {
                    seq_buffer * q = sdp->queries[3 * s + f];

                    qlen = query.aa[3 * s + f].len;
                    q->qlen = qlen;
                    q->qtable = (uint8_t**) xmalloc(qlen * sizeof(uint8_t*));
                    for (int i = 0; i < qlen; i++) {
                        q->qtable[i] = sdp->dprofile + 64 * query.aa[3 * s + f].seq[i];
                    }
                    hearraylen = qlen > hearraylen ? qlen : hearraylen;
                }
    }

    //  fprintf(out, "hearray length = %ld\n", hearraylen);

    sdp->hearray = (uint8_t*) xmalloc(hearraylen * 32);

    long listsize = maxchunksize * sizeof(long);
    if ((symtype == TRANS_DB) || (symtype == TRANS_BOTH))
        listsize *= 6;

    sdp->start_list = (long*) xmalloc(listsize);
    sdp->in_list = (long*) xmalloc(listsize);
    sdp->out_list = (long*) xmalloc(listsize);
    sdp->scores = (long*) xmalloc(listsize);
    sdp->bestpos = (long*) xmalloc(listsize);
    sdp->bestq = (long*) xmalloc(listsize);

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
}


/**
 * Searches for a sequence in the database.
 *
 * TODO implement following:
 *  - search for X sequence with highest count first
 *  - do the alignment afterwards
 *  - return best X sequences/alignments
 *  - - X can be defined by the caller

 * first implementation gets sequences one by one, later with chunks (or just with threads)

 * use a priority queue for keeping the best x sequences

 *
 *  - select right search tool (7, 16, 16s, 63) (based on initialisation)
 *  - selection of DB sequences (use some masking)
 *  -
 *
 * @param query     the query sequence
 * @param count     the number of highest scoring sequences to return
 *
 * @return returns a struct containing all alignments with the highest x scoring
 *          sequences, where is is equal to the value of the parameter count
 */
p_alignment_list search(p_query query, long count) {
    struct search_data sd;

    init_search_buffer(query);

    // TODO select search tool based on init/what is available

    it_init();

    search_init(query, &sd);

    p_minheap heap = minheap_init(count);

    p_sdb_sequence dseq;
    char * qseq;
    long qlen;
    while ((dseq = it_next())) {


        long *hearray = calloc(sizeof(long), 32 * qlen);

        e.score = fullsw(dseq->seq, dseq->seq + dseq->len,
                    qseq, qseq + qlen,
                    hearray,
                    score_matrix_63,
                    gapO, gapE);
        do_search(&sd, query, dseq);

        elem_t e;
        e.seq_info = dseq;

        minheap_add(heap, &e);
    }

    p_alignment_list result = init_alignment_list(heap->count);

    minheap_sort(heap);

    result->length = heap->count;
    for (int i = 0; i < result->length; i++) {

        // TODO call align_sequences()

//        result[i] = NULL; // do alignment and add to result struct
    }

    minheap_exit(heap);

    return result;
}

