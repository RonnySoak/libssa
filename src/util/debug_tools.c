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

#include "debug_tools.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util_sequence.h"
#include "util.h"
#include "../matrices.h"
#include "../query.h"

static int16_t ** matrix = 0;
static int matrix_count = 0;

static size_t matrix_width;
static size_t matrix_height;

static const char * get_symbol_translation() {
    if( (symtype == NUCLEOTIDE) ) {
        return sym_ncbi_nt16u;
    }
    return sym_ncbi_aa;
}

void dbg_init_matrix_data_collection( int bit_width, size_t maxdlen, size_t maxqlen ) {
    if( bit_width == BIT_WIDTH_64 ) {
        matrix_count = 3;
    }
    else if( bit_width == BIT_WIDTH_16 ) {
        matrix_count = 8;
    }
    else if( bit_width == BIT_WIDTH_8 ) {
        matrix_count = 16;
    }
    else {
        ffatal( "Unknown bit width: %d", bit_width );

        matrix_count = 0;
    }

    matrix = xmalloc( matrix_count * sizeof(int16_t *) );

    matrix_width = maxdlen;
    matrix_height = maxqlen;

    for( int i = 0; i < matrix_count; ++i ) {
        matrix[i] = xmalloc( sizeof(int16_t) * maxdlen * maxqlen );

        memset( matrix[i], 0, sizeof(int16_t) * maxdlen * maxqlen );
    }
}

void dbg_add_matrix_data_64( uint8_t q_idx, size_t d_idx, int h, int e, int f ) {
    matrix[0][q_idx * matrix_width + d_idx] = h;
    matrix[1][q_idx * matrix_width + d_idx] = e;
    matrix[2][q_idx * matrix_width + d_idx] = f;
}

void dbg_add_matrix_data_128_16( uint8_t q_idx, size_t d_idx, __m128i value ) {
    int16_t data[matrix_count];
    _mm_storeu_si128( (__m128i *) data, value );

    for( int x = 0; x < matrix_count; ++x ) {
        matrix[x][q_idx * matrix_width + d_idx] = data[x];
    }
}

void dbg_add_matrix_data_128_8( uint8_t q_idx, size_t d_idx, __m128i value ) {
    int8_t data[matrix_count];
    _mm_storeu_si128( (__m128i *) data, value );

    for( int x = 0; x < matrix_count; ++x ) {
        matrix[x][q_idx * matrix_width + d_idx] = data[x];
    }
}

void dbg_add_matrix_data_128_8_sw( uint8_t q_idx, size_t d_idx, __m128i value ) {
    int8_t data[matrix_count];
    _mm_storeu_si128( (__m128i *) data, value );

    for( int x = 0; x < matrix_count; ++x ) {
        matrix[x][q_idx * matrix_width + d_idx] = (int16_t) data[x] + -INT8_MIN;
    }
}

void dbg_add_matrix_data_128_16_sw( uint8_t q_idx, size_t d_idx, __m128i value ) {
    int16_t data[matrix_count];
    _mm_storeu_si128( (__m128i *) data, value );

    for( int x = 0; x < matrix_count; ++x ) {
        matrix[x][q_idx * matrix_width + d_idx] = data[x] + -INT16_MIN;
    }
}

void dbg_add_matrix_data_256_8( uint8_t q_idx, size_t d_idx, __m256i value ) {
    ffatal( "not implemented yet" );
}

void dbg_add_matrix_data_256_16( uint8_t q_idx, size_t d_idx, __m256i value ) {
    ffatal( "not implemented yet" );
}

void dbg_add_matrix_data_256_8_sw( uint8_t q_idx, size_t d_idx, __m256i value ) {
    ffatal( "not implemented yet" );
}

void dbg_add_matrix_data_256_16_sw( uint8_t q_idx, size_t d_idx, __m256i value ) {
    ffatal( "not implemented yet" );
}

static void print_matrix( FILE * f, sequence_t * dseq, int x, char * qseq, int16_t * matrix ) {
    // first line seq1
    fprintf( f, " " );
    for( int i = 0; i < dseq[x].len; i++ ) {
        fprintf( f, "     %c", get_symbol_translation()[(int) dseq[x].seq[i]] );
    }
    fprintf( f, "\n" );

    // rest of matrix and seq2
    for( uint8_t q_id = 0; q_id < matrix_height; q_id++ ) {
        if( qseq ) {
            fprintf( f, " %c", get_symbol_translation()[(int) qseq[q_id]] );
        }

        for( int d_id = 0; d_id < dseq[x].len; d_id++ ) {
            fprintf( f, " %4d ", matrix[q_id * matrix_width + d_id] );
        }
        fprintf( f, "\n" );
    }
    fprintf( f, "\n" );
    fprintf( f, "\n" );
}

void dbg_print_matrices_to_file( int bit_width, char * algorithm, char * qseq, sequence_t * dseq, size_t dseq_count ) {
    char * bit_string = "";

    if( bit_width == BIT_WIDTH_64 ) {
        bit_string = "64_bit";
    }
    else if( bit_width == BIT_WIDTH_16 ) {
        bit_string = "16_bit";
    }
    else if( bit_width == BIT_WIDTH_8 ) {
        bit_string = "8_bit";
    }

    if( dseq_count > matrix_count ) {
        print_info( "\nThere are more database sequences, than matrices: DB %d, M %d\n", dseq_count, matrix_count );
        print_info( "Only %d matrices will be printed.\n", matrix_count );

        dseq_count = matrix_count;
    }

    char * file_name = xmalloc( 40 );
    sprintf( file_name, "debug_output/matrix_%s_%s.txt", bit_string, algorithm );

    FILE *f = fopen( file_name, "a" );
    if( f == NULL ) {
        print_info( "Error opening file!\n" );
        exit( 1 );
    }

    free( file_name );

    for( int x = 0; x < dseq_count; ++x ) {
        fprintf( f, "H-Matrix\n" );
        print_matrix( f, dseq, x, qseq, matrix[x] );

        if( bit_width == BIT_WIDTH_64 ) {
            fprintf( f, "E-Matrix\n" );
            print_matrix( f, dseq, x, qseq, matrix[x + 1] );
            free( matrix[x + 1] );

            fprintf( f, "F-Matrix\n" );
            print_matrix( f, dseq, x, qseq, matrix[x + 2] );
            free( matrix[x + 2] );
        }

        free( matrix[x] );
    }
    free( matrix );

    fclose( f );
}

static char * aligned_sequence_collection_desc = 0;
static p_minheap aligned_sequences = 0;

void dbg_init_aligned_sequence_collecting( char * desc, size_t size ) {
    aligned_sequence_collection_desc = desc;

    aligned_sequences = minheap_init( size );
}

void dbg_add_aligned_sequence( size_t db_id, uint8_t query_id, long score ) {
    if( aligned_sequence_collection_desc ) {
        elem_t e;
        e.query_id = query_id;
        e.db_id = db_id;
        e.score = score;

        minheap_add( aligned_sequences, &e );
    }
}

static int dbg_minheap_compare( const void * a, const void * b ) {
    elem_t * x = (elem_t*) a;
    elem_t * y = (elem_t*) b;

//    int cmp = CMP_ASC( x->score, y->score );
//    if( !cmp ) {
//        cmp = CMP_ASC( x->db_id, y->db_id );
//    }
    return CMP_ASC( x->db_id, y->db_id ); //cmp;
}

static void dbg_minheap_sort( p_minheap m ) {
    qsort( m->array, m->count, sizeof(elem_t), dbg_minheap_compare );
}

void dbg_print_aligned_sequences() {
    if( aligned_sequences ) {
        char * file_name = calloc( 50, 1 );
        sprintf( file_name, "debug_output/aligned_sequences_%s.txt", aligned_sequence_collection_desc );

        FILE * f = fopen( file_name, "a" );
        if( f == NULL ) {
            print_info( "Error opening file!\n" );
            exit( 1 );
        }
        free( file_name );

        dbg_minheap_sort( aligned_sequences );

        for( int i = 0; i < aligned_sequences->count; ++i ) {
            elem_t data = aligned_sequences->array[i];

            fprintf( f, "DB-ID %ld, query-ID: %d, score: %ld\n", data.db_id, data.query_id, data.score );
        }
        fclose( f );
    }

    minheap_exit( aligned_sequences );
    aligned_sequences = 0;

    if( aligned_sequence_collection_desc )
        free( aligned_sequence_collection_desc );
    aligned_sequence_collection_desc = 0;
}

void dbg_mm_print_8u( char * desc, __m128i x ) {
    uint8_t * y = (uint8_t*) &x;

    print_info( "%s: ", desc );

    for( int i = 0; i < 16; i++ )
        print_info( "%s%3d", (i > 0 ? " " : ""), y[15 - i] );
    print_info( "\n" );
}

void dbg_mm_print_8s( char * desc, __m128i x ) {
    int8_t* y = (int8_t*) &x;

    print_info( "%s: ", desc );

    for( int i = 0; i < 16; i++ ) {
        print_info( "%s%3d", (i > 0 ? " " : ""), y[15 - i] );
    }
    print_info( "\n" );
}

void dbg_mm_print_16u( char * desc, __m128i x ) {
    uint16_t * y = (uint16_t*) &x;

    print_info( "%s: ", desc );

    for( int i = 0; i < 8; i++ )
        print_info( "%s%6d", (i > 0 ? " " : ""), y[7 - i] );
    print_info( "\n" );
}

void dbg_mm256_print_16u( char * desc, __m256i x ) {
    uint16_t * y = (uint16_t*) &x;

    print_info( "%s: ", desc );

    for( int i = 0; i < 16; i++ )
        print_info( "%s%3d", (i > 0 ? " " : ""), y[15 - i] );
    print_info( "\n" );
}

void dbg_mm_print_16s( char * desc, __m128i x ) {
    int16_t * y = (int16_t*) &x;

    print_info( "%s: ", desc );

    for( int i = 0; i < 8; i++ ) {
        print_info( "%s%3d", (i > 0 ? " " : ""), y[7 - i] );
    }
    print_info( "\n" );
}

void dbg_mm256_print_16s( char * desc, __m256i x ) {
    int16_t * y = (int16_t*) &x;

    print_info( "%s: ", desc );

    for( int i = 0; i < 16; i++ )
        print_info( "%s%3d", (i > 0 ? " " : ""), y[15 - i] );
    print_info( "\n" );
}

void dbg_mm256_print_8s( char * desc, __m256i x ) {
    int8_t * y = (int8_t*) &x;

    print_info( "%s: ", desc );

    for( int i = 0; i < 32; i++ )
        print_info( "%s%3d", (i > 0 ? " " : ""), y[31 - i] );
    print_info( "\n" );
}

void dbg_mm_print_sw_16s( char * desc, __m128i x ) {
    int16_t * y = (int16_t*) &x;

    print_info( "%s: ", desc );

    for( int i = 0; i < 8; i++ ) {
        print_info( "%s%3d", (i > 0 ? " " : ""), y[7 - i] + -INT16_MIN );
    }
    print_info( "\n" );
}

void dbg_dprofile_dump_16( int16_t * dprofile, int cdepth, int channels ) {
    const char * s = sym_ncbi_nt16u;
    print_info( "\ndprofile (NUC):\n" );
    for( int i = 0; i < SCORE_MATRIX_DIM; i++ ) {
        print_info( "%c: ", s[i] );
        for( int k = 0; k < cdepth; k++ ) {
            print_info( "[" );
            for( int j = 0; j < channels; j++ )
                print_info( " %2d", dprofile[channels * cdepth * i + channels * k + j] );
            print_info( "]" );
        }
        print_info( "\n" );
    }
}

void dbg_dprofile_dump_8( int8_t * dprofile, int cdepth, int channels ) {
    const char * s = sym_ncbi_nt16u;
    print_info( "\ndprofile (NUC):\n" );
    for( int i = 0; i < SCORE_MATRIX_DIM; i++ ) {
        print_info( "%c: ", s[i] );
        for( int k = 0; k < cdepth; k++ ) {
            print_info( "[" );
            for( int j = 0; j < channels; j++ )
                print_info( " %2d", dprofile[channels * cdepth * i + channels * k + j] );
            print_info( "]" );
        }
        print_info( "\n" );
    }
}

void dbg_dumpscorematrix_16( int16_t * m ) {
    for( int i = 0; i < SCORE_MATRIX_DIM; i++ ) {
        print_info( "%2d %c", i, sym_ncbi_nt16u[i] );
        for( int j = 0; j < SCORE_MATRIX_DIM; j++ )
            print_info( " %2d", m[SCORE_MATRIX_DIM * i + j] );
        print_info( "\n" );
    }
}

void dbg_dumpscorematrix_8( int8_t * m ) {
    for( int i = 0; i < SCORE_MATRIX_DIM; i++ ) {
        print_info( "%2d %c", i, sym_ncbi_nt16u[i] );
        for( int j = 0; j < SCORE_MATRIX_DIM; j++ )
            print_info( " %2d", m[SCORE_MATRIX_DIM * i + j] );
        print_info( "\n" );
    }
}
