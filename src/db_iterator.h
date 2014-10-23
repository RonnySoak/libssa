/*
 * db_iterator.h
 *
 *  Created on: Oct 12, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef DB_ITERATOR_H_
#define DB_ITERATOR_H_

#include "libssa_datatypes.h"

void it_free();

void it_init(unsigned long size);

p_seqinfo it_get_sequence(unsigned long id);

sequence it_translate_sequence(p_seqinfo info, int f, int s);

/**
 * Returns the next sequence from the database. Translates the DB sequences, if
 * necessary, and returns the translated sequences one by one.
 *
 * @return the next sequence in the DB, or NULL if none is left
 */
p_sdb_sequence it_next_sequence();

void it_free_sequence(p_sdb_sequence seq);

void it_free_chunk(p_db_chunk chunk);

void reset_chunk_counter();

p_db_chunk it_next_chunk();

#endif /* DB_ITERATOR_H_ */
