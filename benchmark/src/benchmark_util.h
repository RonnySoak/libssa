/*
 * benchmark_util.h
 *
 *  Created on: Apr 3, 2015
 *      Author: Jakob Frielingsdorf
 */

#ifndef BENCHMARK_BENCHMARK_UTIL_H_
#define BENCHMARK_BENCHMARK_UTIL_H_

#include <stdio.h>

#include "../../src/libssa.h"

#define SW 0
#define NW 1

#define SIMD_DESC(s) ( (s == COMPUTE_ON_AVX2) ? "AVX2" : "SSE41" )
#define TYPE_DESC(t) ( (t == SW) ? "SW" : "NW" )

double run_alignment_times( int type, p_query query, size_t hit_count, int bit_width, int internal_iterations );

double run_alignment( int type, p_query query, size_t hit_count, int bit_width );

char* concat( char * s1, char * s2 );

p_query read_query( char * query_file );

void log_to_file( FILE * f, const char * format, ... );

FILE * open_log_file( char * desc );

void run_and_log_n_times( FILE * f, int type, p_query query, int hit_count, int bit_width, int iterations );

#endif /* BENCHMARK_BENCHMARK_UTIL_H_ */
