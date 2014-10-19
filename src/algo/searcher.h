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

void s_free();

void * s_search(void *);

#endif /* SEARCHER_H_ */
