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
