/*
 * db_iterator.h
 *
 *  Created on: Oct 12, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef DB_ITERATOR_H_
#define DB_ITERATOR_H_

#include "libssa_datatypes.h"

void it_exit();

void it_init( size_t size );

void it_free_sequence( p_sdb_sequence seq );

p_db_chunk it_alloc_chunk( size_t size );
p_db_chunk it_init_new_chunk();

void it_next_chunk( p_db_chunk chunk );

void it_free_chunk( p_db_chunk chunk );

void it_reset_chunk_counter();

#endif /* DB_ITERATOR_H_ */
