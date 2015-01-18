/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_16_H_
#define SEARCH_16_H_

#include <stdint.h>
#include <immintrin.h>

#include "../../libssa_datatypes.h"
#include "../../util/minheap.h"

struct s8info {
    __m128i matrix[32];
    __m128i * hearray;
    __m128i * dprofile;
    __m128i ** qtable;

    int qlen;
    int maxqlen;
    int maxdlen;

    int8_t penalty_gap_open;
    int8_t penalty_gap_extension;
};

typedef struct s8info * p_s8info;

#endif /* SEARCH_16_H_ */
