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

#ifndef MINHEAP_H_
#define MINHEAP_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t db_id;        // id of the DB sequence
    uint8_t db_frame;      // strand of the DB sequence
    uint8_t db_strand;     // frame the DB sequence
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
