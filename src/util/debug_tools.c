/*
 * debug_tools.c
 *
 *  Created on: Jan 28, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "debug_tools.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util_sequence.h"
#include "util.h"
#include "linked_list.h"
#include "../matrices.h"

static int16_t ** matrix = 0;
static int matrix_count = 0;

static int matrix_width;
static int matrix_height;
static const char * symbol_translation;

void dbg_init_matrix_data_collection( int bit_width, int maxdlen, int maxqlen ) {
    if( bit_width == BIT_WIDTH_64 ) {
        matrix_count = 1;
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
    symbol_translation = sym_ncbi_aa;

    for( int i = 0; i < matrix_count; ++i ) {
        matrix[i] = xmalloc( sizeof(int16_t) * maxdlen * maxqlen );

        memset( matrix[i], 0, sizeof(int16_t) * maxdlen * maxqlen );
    }
}

void dbg_add_matrix_data_64( int q_idx, int d_idx, int value ) {
    matrix[0][q_idx * matrix_width + d_idx] = value;
}

void dbg_add_matrix_data_128_16( int q_idx, int d_idx, __m128i value ) {
    int16_t data[matrix_count];
    _mm_storeu_si128( (__m128i *) data, value );

    for( int x = 0; x < matrix_count; ++x ) {
        matrix[x][q_idx * matrix_width + d_idx] = data[x];
    }
}

void dbg_add_matrix_data_128_8( int q_idx, int d_idx, __m128i value ) {
    int8_t data[matrix_count];
    _mm_storeu_si128( (__m128i *) data, value );

    for( int x = 0; x < matrix_count; ++x ) {
        matrix[x][q_idx * matrix_width + d_idx] = data[x];
    }
}

void dbg_add_matrix_data_128_8_sw( int q_idx, int d_idx, __m128i value ) {
    int8_t data[matrix_count];
    _mm_storeu_si128( (__m128i *) data, value );

    for( int x = 0; x < matrix_count; ++x ) {
        matrix[x][q_idx * matrix_width + d_idx] = (int16_t) data[x] + -INT8_MIN;
    }
}

void dbg_add_matrix_data_128_16_sw( int q_idx, int d_idx, __m128i value ) {
    int16_t data[matrix_count];
    _mm_storeu_si128( (__m128i *) data, value );

    for( int x = 0; x < matrix_count; ++x ) {
        matrix[x][q_idx * matrix_width + d_idx] = data[x] + -INT16_MIN;
    }
}

void dbg_print_matrices_to_file( int bit_width, char * algorithm, char * qseq, sequence * dseq, int dseq_count ) {
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
        printf( "\nThere are more database sequences, than matrices: DB %d, M %d\n", dseq_count, matrix_count );
        printf( "Only %d matrices will be printed.\n", matrix_count );

        dseq_count = matrix_count;
    }

    for( int x = 0; x < dseq_count; ++x ) {
        char * file_name = xmalloc( 40 );
        sprintf( file_name, "debug_output/matrix_%s_%s.txt", bit_string, algorithm );

        FILE *f = fopen( file_name, "a" );
        if( f == NULL ) {
            printf( "Error opening file!\n" );
            exit( 1 );
        }

        free( file_name );
        // first line seq1
        fprintf( f, " " );
        for( int i = 0; i < dseq[x].len; i++ ) {
            fprintf( f, "     %c", symbol_translation[(int) dseq[x].seq[i]] );
        }
        fprintf( f, "\n" );

        // rest of matrix and seq2
        for( int q_id = 0; q_id < matrix_height; q_id++ ) {
            if( qseq ) {
                fprintf( f, " %c", symbol_translation[(int) qseq[q_id]] );
            }

            for( int d_id = 0; d_id < dseq[x].len; d_id++ ) {
                fprintf( f, " %4d ", matrix[x][q_id * matrix_width + d_id] );
            }
            fprintf( f, "\n" );
        }
        fprintf( f, "\n" );
        fprintf( f, "\n" );

        fclose( f );

        free( matrix[x] );
    }
    free( matrix );
}

static char * aligned_sequence_collection_desc = 0;
static p_minheap aligned_sequences = 0;

void dbg_init_aligned_sequence_collecting( char * desc, int size ) {
    aligned_sequence_collection_desc = desc;

    aligned_sequences = minheap_init( size );
}

void dbg_add_aligned_sequence( unsigned long db_id, int query_id, long score ) {
    if( aligned_sequence_collection_desc ) {
        elem_t * e = xmalloc( sizeof(elem_t) );
        e->query_id = query_id;
        e->db_id = db_id;
        e->score = score;

        minheap_add( aligned_sequences, e );
    }
}

void dbg_print_aligned_sequences() {
    if( aligned_sequences ) {
        char * file_name = calloc( 50, 1 );
        sprintf( file_name, "debug_output/aligned_sequences_%s.txt", aligned_sequence_collection_desc );

        FILE * f = fopen( file_name, "a" );
        if( f == NULL ) {
            printf( "Error opening file!\n" );
            exit( 1 );
        }
        free( file_name );

        minheap_sort( aligned_sequences );

        for (int i = 0; i < aligned_sequences->count; ++i) {
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

    printf( "%s: ", desc );

    for( int i = 0; i < 16; i++ )
        printf( "%s%6d", (i > 0 ? " " : ""), y[15 - i] );
    printf( "\n" );
}

void dbg_mm_print_8s( char * desc, __m128i x ) {
    int8_t* y = (int8_t*) &x;

    printf( "%s: ", desc );

    for( int i = 0; i < 16; i++ ) {
        printf( "%s%2d", (i > 0 ? " " : ""), y[15 - i] );
    }
    printf( "\n" );
}

void dbg_mm_print_16u( char * desc, __m128i x ) {
    uint16_t * y = (uint16_t*) &x;

    printf( "%s: ", desc );

    for( int i = 0; i < 8; i++ )
        printf( "%s%6d", (i > 0 ? " " : ""), y[7 - i] );
    printf( "\n" );
}

void dbg_mm_print_16s( char * desc, __m128i x ) {
    int16_t * y = (int16_t*) &x;

    printf( "%s: ", desc );

    for( int i = 0; i < 8; i++ ) {
        printf( "%s%2d", (i > 0 ? " " : ""), y[7 - i] );
    }
    printf( "\n" );
}

void dbg_mm_print_sw_16s( char * desc, __m128i x ) {
    int16_t * y = (int16_t*) &x;

    printf( "%s: ", desc );

    for( int i = 0; i < 8; i++ ) {
        printf( "%s%2d", (i > 0 ? " " : ""), y[7 - i] + -INT16_MIN );
    }
    printf( "\n" );
}

void dbg_dprofile_dump16( int16_t * dprofile, int cdepth, int channels ) {
    const char * s = sym_ncbi_nt16u;
    printf( "\ndprofile:\n" );
    for( int i = 0; i < 16; i++ ) {
        printf( "%c: ", s[i] );
        for( int k = 0; k < cdepth; k++ ) {
            printf( "[" );
            for( int j = 0; j < channels; j++ )
                printf( " %3d", dprofile[channels * cdepth * i + channels * k + j] );
            printf( "]" );
        }
        printf( "\n" );
    }
}

void dbg_dumpscorematrix( int16_t * m ) {
    for( int i = 0; i < 16; i++ ) {
        printf( "%2d %c", i, sym_ncbi_nt16u[i] );
        for( int j = 0; j < 16; j++ )
            printf( " %2d", m[16 * i + j] );
        printf( "\n" );
    }
}
