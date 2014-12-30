/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_H_
#define SEARCH_H_

#include <stdint.h>

#include "../libssa_datatypes.h"

// both are filled from libssa.c
uint8_t gapO;
uint8_t gapE;

int64_t full_sw(sequence * dseq, sequence * qseq, int64_t * hearray );

int64_t full_nw(sequence * dseq, sequence * qseq, int64_t * hearray );

int64_t full_nw_sellers(sequence * dseq, sequence * qseq, int64_t * hearray );

#endif /* SEARCH_H_ */
