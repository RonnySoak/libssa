/*
 * check_liborsa.h
 *
 *  Created on: 20 Jul 2014
 *      Author: kaos
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
void addLinkedListTC( Suite *s );
void addThreadPoolTC( Suite *s );
void addQueryTC( Suite *s );
void addMatricesTC( Suite *s );
void addUtilSequenceTC( Suite *s );
void addDBIteratorTC( Suite *s );
void addCigarTC( Suite *s );
void addAlignTC( Suite *s );
void add_sw_63_TC( Suite *s );
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
void addSearcher63TC( Suite *s );
void addSearcherTC( Suite *s );
void addManagerTC( Suite *s );
void addAlignerTC( Suite *s );
void addLibssaTC( Suite *s );

// some utility functions
void dbg_print( int n, void* param );
void print_sequence( char* desc, int8_t* seq, size_t len );
void print_sequence2( char* desc, sequence seq );
char* concat( char *s1, char *s2 );

#endif /* CHECK_LIBORSA_H_ */

