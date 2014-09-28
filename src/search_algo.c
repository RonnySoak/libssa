/*
 * search_algo.c
 *
 *  Created on: Sep 21, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "util.h"

extern long fullsw(char * dseq,
        char * dend,
        char * qseq,
        char * qend,
        long * hearray,
        long * score_matrix,
        uint8_t gapopenextend,
        uint8_t gapextend);

uint8_t gapO;
uint8_t gapE;

// TODO move to libssa_datatypes.h
struct alignment_list {
    p_alignment* alignments;
    long length;
}

typedef struct alignment_list p_alignment_list;

p_alignment_list init_alignment_list(long length) {
    p_alignment_list result = (p_alignment_list)xmalloc(sizeof(struct alignment_list));
    
    for (int i = 0; i < length; i++ ) {
        // TODO how do I get a contiguous memory space
        
        // init each alignment
    }
    
    return result;
}

void free_alignment_list(p_alignment_list list) {
    if (list) {
        for (int i = 0; i < list.length; i++ ) {
            // TODO free alignment
            
            free(list->alignments[i]);
        }
        free(list);
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

    // TODO select search tool based on inti/what is available
    
    // seq = get DB sequence
    // while (seq) {
    
    // long score = do search
    
    // if score > lowest in prio queue
    //      add score to prio queue and remove lowest
    // }
    
    p_alignment_list result = init_alignment_list( /* length of prio queue */ );

    result.length = 0;// length of prio queue
    for (int i = 0; i < result.length; i++) {// for each sequence in prio queue
        result[i] = NULL; // do alignment and add to result struct
    }

    return result;
}


