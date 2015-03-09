/*
 * linked_list.h
 *
 *  Created on: Jan 30, 2015
 *      Author: Jakob Frielingsdorf
 */

#ifndef SRC_UTIL_LINKED_LIST_H_
#define SRC_UTIL_LINKED_LIST_H_

#include <stddef.h>

struct node_t {
    void * data;

    struct node_t * next;
    struct node_t * previous;
};

typedef struct node_t * p_node;

void ll_wind_back( p_node * node );

void ll_clear( p_node * node );

p_node ll_init( void * data );

void ll_push( p_node * current, void * data );

void * ll_pop( p_node * current );

size_t ll_size( p_node node );

void ll_print( p_node * node );

#endif /* SRC_UTIL_LINKED_LIST_H_ */
