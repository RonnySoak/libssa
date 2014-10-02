/*
 * internal_datatypes.h
 *
 *  Created on: Oct 1, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef ALGO_DATATYPES_H_
#define ALGO_DATATYPES_H_

#include "util.h"

#include "util/minheap.h"
#include "libssa_datatypes.h"

struct result_t {
    p_minheap heap;
    unsigned long chunk_count;
    unsigned long seq_count;
};
typedef struct result_t * p_result;

struct db_chunk {
    p_sdb_sequence* seq;
    unsigned long size;
};
typedef struct db_chunk * p_db_chunk;

typedef struct {
    int8_t* qtable;
    unsigned long qlen;
    int strand;
    int frame;
} seq_buffer;

struct search_data {
    struct db_thread_s * dbt;
    struct db_thread_s * dbta[8];

    uint8_t * dprofile;
    uint8_t * hearray;

    seq_buffer queries[6];
    int q_count; // max 6

    long qstrand1, qstrand2, qframe1, qframe2;
};

typedef struct search_data * p_search_data;

#endif /* ALGO_DATATYPES_H_ */
