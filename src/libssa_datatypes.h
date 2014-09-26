/*
 * libssa_datatypes.h
 *
 *  Created on: Sep 3, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef LIBSSA_DATATYPES_H_
#define LIBSSA_DATATYPES_H_

#include "libssa_extern_db.h"

/** @typedef    structure of a sequence
 *
 * @field seq   the sequence
 * @field len   length of the sequence
 */
struct sequence {
  char * seq;
  long len;
};

/** @typedef    structure of a sequence
 *
 * @field seq   the sequence
 * @field len   length of the sequence
 */
typedef struct sdb_sequence {
    p_seqinfo info;
    int strand;
    int frame;
    char* seq;
    long len;
} sdb_sequence;

typedef struct sdb_sequence* p_sdb_sequence;

/** @typedef    structure of the query profile
 *
 * @field nt    strands of the sequence
 * @field aa    reading frames of the sequence
 * @field description   TODO ...
 */
struct _query {
  struct sequence nt[2]; /* 2 strands for nucleotides */
  struct sequence aa[6]; /* 6 reading frames  for amino acids */
  char * description;
  long dlen;
  const char * map;
  const char * sym;
};

#endif /* LIBSSA_DATATYPES_H_ */
