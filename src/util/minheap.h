/*
 * minheap.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef MINHEAP_H_
#define MINHEAP_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t db_id;        // id of the DB sequence
    uint8_t dframe;      // strand of the DB sequence
    uint8_t dstrand;     // frame the DB sequence
    uint8_t query_id;    // id of the compared query in seq_buffer of search_data
    long score;          // score of the alignment
} elem_t;

typedef struct {
    size_t alloc;
    size_t count;
    elem_t * array;
} minheap_t;

typedef minheap_t * p_minheap;

p_minheap minheap_init( size_t size );

void minheap_add( p_minheap m, elem_t * n );

void minheap_sort( p_minheap m );

void minheap_exit( p_minheap m );

#endif /* MINHEAP_H_ */
