/*
 * libssa_datatypes.h
 *
 *  Created on: Sep 3, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef LIBSSA_DATATYPES_H_
#define LIBSSA_DATATYPES_H_

#include "libssa_extern_db.h"
#include "libssa_datatypes.h"

#include "util.h"
#include "util/minheap.h"

/** @typedef    structure of a sequence
 *
 * @field seq   the sequence
 * @field len   length of the sequence
 */
typedef struct {
  char * seq;
  unsigned long len;
} sequence;

struct search_result {
    p_minheap heap;
    unsigned long chunk_count;
    unsigned long seq_count;
};
typedef struct search_result * p_search_result;

typedef struct {
    sequence seq;
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
};
typedef struct search_data * p_search_data;

/** @typedef    structure of a sequence
 *
 * @field seq   the sequence
 * @field len   length of the sequence
 */
typedef struct {
    p_seqinfo info;
    sequence seq;
    int strand;
    int frame;
} sdb_sequence;
typedef sdb_sequence* p_sdb_sequence;

struct db_chunk {
    p_sdb_sequence* seq;
    unsigned long size;
};
typedef struct db_chunk * p_db_chunk;

/** @typedef    structure of the query profile
 *
 * @field nt    strands of the sequence
 * @field aa    reading frames of the sequence
 * @field description   TODO ...
 */
struct _query {
  sequence nt[2]; /* 2 strands for nucleotides */
  sequence aa[6]; /* 6 reading frames  for amino acids */
  char * header;
  unsigned long headerlen;
  const char * map;
  const char * sym;
};

#endif /* LIBSSA_DATATYPES_H_ */
