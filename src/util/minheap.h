/*
 * minheap.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef MINHEAP_H_
#define MINHEAP_H_

#include <stddef.h>

typedef struct topscore {
    unsigned long db_id; // id of the DB sequence
    int dframe;          // strand of the DB sequence
    int dstrand;         // frame the DB sequence
    int query_id;        // id of the compared query in seq_buffer of search_data
    long score;
} elem_t;

struct minheap_t {
    size_t alloc;
    size_t count;
    elem_t * array;
};

typedef struct minheap_t * p_minheap;

p_minheap minheap_init( size_t size );

void minheap_add( p_minheap m, elem_t * n );

void minheap_sort( p_minheap m );

void minheap_exit( p_minheap m );

#endif /* MINHEAP_H_ */
