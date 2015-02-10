/*
 * debug_tools.h
 *
 *  Created on: Jan 28, 2015
 *      Author: Jakob Frielingsdorf
 */

#ifndef SRC_UTIL_DEBUG_TOOLS_H_
#define SRC_UTIL_DEBUG_TOOLS_H_

#include <stdint.h>
#include <immintrin.h>

#include "../libssa_datatypes.h"

//#define DBG_COLLECT_MATRIX
//#define DBG_COLLECT_ALIGNED_DB_SEQUENCES

void dbg_init_matrix_data_collection( int bit_width, int maxdlen, int maxqlen );

void dbg_add_matrix_data_64( int q_idx, int d_idx, int value );
void dbg_add_matrix_data_128_8( int q_idx, int d_idx, __m128i value );
void dbg_add_matrix_data_128_16( int q_idx, int d_idx, __m128i value );
void dbg_add_matrix_data_128_8_sw( int q_idx, int d_idx, __m128i value );
void dbg_add_matrix_data_128_16_sw( int q_idx, int d_idx, __m128i value );

void dbg_print_matrices_to_file( int bit_width, char * algorithm, char * qseq, sequence * dseq, int dseq_count );

void dbg_init_aligned_sequence_collecting( char * desc, int size );
void dbg_add_aligned_sequence( unsigned long db_id, int query_id, long score );
void dbg_print_aligned_sequences();

void dbg_mm_print_8u( char * desc, __m128i x );
void dbg_mm_print_8s( char * desc, __m128i x );

void dbg_mm_print_16u( char * desc, __m128i x );
void dbg_mm256_print_16u( char * desc, __m256i x );
void dbg_mm_print_16s( char * desc, __m128i x );
void dbg_mm256_print_16s( char * desc, __m256i x );
void dbg_mm_print_sw_16s( char * desc, __m128i x );

void dbg_dprofile_dump_16( int16_t * dprofile, int cdepth, int channels );
void dbg_dprofile_dump_8( int8_t * dprofile, int cdepth, int channels );

void dbg_dumpscorematrix_16( int16_t * m );
void dbg_dumpscorematrix_8( int8_t * m );

#endif /* SRC_UTIL_DEBUG_TOOLS_H_ */
