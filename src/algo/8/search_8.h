/*
 * search.h
 *
 *  Created on: Oct 18, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SEARCH_8_H_
#define SEARCH_8_H_

#include <stdint.h>
#include <immintrin.h>

#include "../16/search_16.h"
#include "../../libssa_datatypes.h"
#include "../../util/minheap.h"

#define CDEPTH_8_BIT 4
#define CHANNELS_8_BIT_SSE (128 / 8)
#define CHANNELS_8_BIT_AVX (256 / 8)

struct s8query;
typedef struct s8query * p_s8query;

struct s8info;
typedef struct s8info * p_s8info;

void search_8_init_algo( int search_type );

p_s8info search_8_init( p_search_data sdp );
void search_8_exit( p_s8info s );

void search_8( p_db_chunk chunk, p_search_data sdp, p_search_result res );

#endif /* SEARCH_8_H_ */
