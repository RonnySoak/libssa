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

#ifndef CHECK_LIBORSA_H_
#define CHECK_LIBORSA_H_

#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/libssa_datatypes.h"

void addCPUConfigTC( Suite *s );
void addUtilTC( Suite *s );
void addMinHeapTC( Suite *s );
void addThreadPoolTC( Suite *s );
void addQueryTC( Suite *s );
void addMatricesTC( Suite *s );
void addUtilSequenceTC( Suite *s );
void addDBAdapterTC( Suite *s );
void addCigarTC( Suite *s );
void addAlignTC( Suite *s );
void add_sw_64_TC( Suite *s );
void add_16_simd_utilities_TC( Suite * s );
void add_8_simd_utilities_TC( Suite * s );
void add_sw_16_SSE2_TC( Suite *s );
void add_nw_16_SSE2_TC( Suite *s );
void add_sw_16_AVX2_TC( Suite *s );
void add_nw_16_AVX2_TC( Suite *s );
void add_sw_8_SSE41_TC( Suite *s );
void add_nw_8_SSE41_TC( Suite *s );
void add_sw_8_AVX2_TC( Suite *s );
void add_nw_8_AVX2_TC( Suite *s );
void addSearcher64TC( Suite *s );
void addSearcherTC( Suite *s );
void addManagerTC( Suite *s );
void addAlignerTC( Suite *s );
void addLibssaTC( Suite *s );
void addBiggerDatabasesTC( Suite *s );

// some utility functions
void dbg_print( int n, void* param );
void print_sequence( char* desc, int8_t* seq, size_t len );
void print_sequence2( char* desc, sequence_t seq );
char* concat( char *s1, char *s2 );

#endif /* CHECK_LIBORSA_H_ */

