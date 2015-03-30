/*
 Copyright (C) 2014-2015 Jakob Frielingsdorf

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Contact: Jakob Frielingsdorf <jfrielingsdorf@gmail.com>
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
    uint8_t strand;
    uint8_t frame;
} seq_buffer_t;

typedef struct {
    size_t maxqlen;

    seq_buffer_t queries[6];
    uint8_t q_count; // max 6
} search_data_t;
typedef search_data_t * p_search_data;

typedef struct {
    elem_t * result_sequence_pairs;
    size_t pair_count;

    seq_buffer_t queries[6];
    size_t q_count;

    int search_type;
} alignment_data_t;
typedef alignment_data_t * p_alignment_data;

/** @typedef    structure of a sequence
 *
 * @field seq   the sequence
 * @field len   length of the sequence
 */
typedef struct {
    size_t ID;
    sequence_t seq;
    uint8_t strand;
    uint8_t frame;
} sdb_sequence_t;
typedef sdb_sequence_t * p_sdb_sequence;

typedef struct {
    p_sdb_sequence * seq;
    size_t size;
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
