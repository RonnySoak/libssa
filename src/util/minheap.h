/*
 * minheap.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef MINHEAP_H_
#define MINHEAP_H_

typedef struct topscore {
    void* db_seq;
    void* query;
    long score;
} elem_t;

struct minheap_s {
    int alloc;
    int count;
    elem_t * array;
};

typedef struct minheap_s * p_minheap;

p_minheap minheap_init(int size);

void minheap_add(p_minheap m, elem_t * n);

void minheap_sort(p_minheap m);

void minheap_exit(p_minheap m);

#endif /* MINHEAP_H_ */
