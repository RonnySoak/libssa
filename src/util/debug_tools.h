/*
 * debug_tools.h
 *
 *  Created on: Jan 28, 2015
 *      Author: Jakob Frielingsdorf
 */

#ifndef SRC_UTIL_DEBUG_TOOLS_H_
#define SRC_UTIL_DEBUG_TOOLS_H_

#include "stdint.h"
#include "immintrin.h"

void dbg_mm_print_u( char * desc, __m128i x );

void dbg_mm_print_s( char * desc, __m128i x );

void dbg_dprofile_dump16( int16_t * dprofile, int cdepth, int channels );

void dbg_dumpscorematrix( int16_t * m );

#endif /* SRC_UTIL_DEBUG_TOOLS_H_ */
