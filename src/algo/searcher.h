/*
 * searcher.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCHER_H_
#define SEARCHER_H_

#include "../libssa_datatypes.h"

extern uint8_t gapO;
extern uint8_t gapE;

void s_init_search_algo(
        long (*algo_p)(sequence *, sequence *, int64_t *, int64_t *));

void s_init(p_search_data data, long hit_count);

void s_free();

p_search_result s_search();

#endif /* SEARCHER_H_ */
