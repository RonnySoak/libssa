/*
 * libssa_datatypes.h
 *
 *  Created on: Sep 3, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef LIBSSA_DATATYPES_H_
#define LIBSSA_DATATYPES_H_

/** @typedef    structure of a sequence
 *
 * @field seq   the sequence
 * @field len   length of the sequence
 */
struct sequence {
  char * seq;
  long len;
};

/** @typedef    structure of the query profile
 *
 * @field nt    strands of the sequence
 * @field aa    reading frames of the sequence
 * @field description   TODO ...
 */
struct _query {
  struct sequence nt[2]; /* 2 strands */
  struct sequence aa[6]; /* 6 frames */
  char * description;
  long dlen;
  long strands;
  char * map;
  const char * sym;
};

#endif /* LIBSSA_DATATYPES_H_ */
