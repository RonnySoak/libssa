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

#ifndef SRC_UTIL_DEBUG_TOOLS_H_
#define SRC_UTIL_DEBUG_TOOLS_H_

#include <stdint.h>
#include <immintrin.h>

#include "../libssa_datatypes.h"

//#define DBG_COLLECT_MATRIX
//#define DBG_COLLECT_ALIGNED_DB_SEQUENCES

void dbg_init_matrix_data_collection( int bit_width, size_t maxdlen, size_t maxqlen );

void dbg_add_matrix_data_64( uint8_t q_idx, size_t d_idx, int h, int e, int f );
void dbg_add_matrix_data_128_8( uint8_t q_idx, size_t d_idx, __m128i value );
void dbg_add_matrix_data_128_16( uint8_t q_idx, size_t d_idx, __m128i value );
void dbg_add_matrix_data_128_8_sw( uint8_t q_idx, size_t d_idx, __m128i value );
void dbg_add_matrix_data_128_16_sw( uint8_t q_idx, size_t d_idx, __m128i value );

void dbg_add_matrix_data_256_8( uint8_t q_idx, size_t d_idx, __m256i value );
void dbg_add_matrix_data_256_16( uint8_t q_idx, size_t d_idx, __m256i value );
void dbg_add_matrix_data_256_8_sw( uint8_t q_idx, size_t d_idx, __m256i value );
void dbg_add_matrix_data_256_16_sw( uint8_t q_idx, size_t d_idx, __m256i value );

void dbg_print_matrices_to_file( int bit_width, char * algorithm, char * qseq, sequence_t * dseq, size_t dseq_count );

void dbg_init_aligned_sequence_collecting( char * desc, size_t size );
void dbg_add_aligned_sequence( size_t db_id, uint8_t query_id, long score );
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
