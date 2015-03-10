/*
 * libssa_datatypes.h
 *
 *  Created on: Sep 3, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef LIBSSA_DATATYPES_H_
#define LIBSSA_DATATYPES_H_

#include "libssa_extern_db.h"
#include "libssa.h"

#include <stddef.h>

#include "util/minheap.h"

/** @typedef    structure of a sequence
 *
 * @field seq   the sequence
 * @field len   length of the sequence
 */
typedef struct {
    char * seq;
    size_t len;
} sequence_t;

typedef struct {
    p_minheap heap;
    size_t chunk_count;
    size_t seq_count;
    size_t overflow_8_bit_count;
    size_t overflow_16_bit_count;
} search_result_t;
typedef search_result_t * p_search_result;

typedef struct {
    sequence_t seq;
    int strand;
    int frame;
} seq_buffer_t;

typedef struct {
    size_t maxqlen;

    seq_buffer_t queries[6];
    int q_count; // max 6
} search_data_t;
typedef search_data_t * p_search_data;

typedef struct {
    elem_t * result_sequence_pairs;
    size_t pair_count;

    seq_buffer_t queries[6];
    size_t q_count;

    void (*align_function)( p_alignment );
} alignment_data_t;
typedef alignment_data_t * p_alignment_data;

/** @typedef    structure of a sequence
 *
 * @field seq   the sequence
 * @field len   length of the sequence
 */
typedef struct {
    unsigned long ID;
    sequence_t seq;
    int strand;
    int frame;
} sdb_sequence_t;
typedef sdb_sequence_t * p_sdb_sequence;

typedef struct {
    p_sdb_sequence * seq;
    size_t size; // TODO change to long only
    size_t fill_pointer;
} db_chunk_t;
typedef db_chunk_t * p_db_chunk;

/** @typedef    structure of the query profile
 *
 * @field nt    strands of the sequence
 * @field aa    reading frames of the sequence
 * @field description   TODO ...
 */
typedef struct _query {
    sequence_t nt[2]; /* 2 strands for nucleotides */
    sequence_t aa[6]; /* 6 reading frames  for amino acids */
    char * header;
    size_t headerlen;
    const char * map;
    const char * sym;
} query_t;

#endif /* LIBSSA_DATATYPES_H_ */
