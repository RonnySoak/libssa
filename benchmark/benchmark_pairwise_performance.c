/*
 * chunk_benchmark.c
 *
 *  Created on: Feb 25, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "../src/libssa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define SW 0
#define NW 1

#define SIMD_DESC(s) ( (s == COMPUTE_ON_AVX2) ? "AVX2" : "SSE41" )
#define TYPE_DESC(t) ( (t == SW) ? "SW" : "NW" )

static double run_alignment( p_alignment_list (*align_func)( p_query, size_t, int, int ), p_query query,
        size_t hit_count, int bit_width, int internal_iterations ) {

    struct timeval start;
    struct timeval finish;

    gettimeofday( &start, NULL );

    for( int i = 0; i < internal_iterations; ++i ) {
        p_alignment_list alist = align_func( query, hit_count, bit_width, COMPUTE_SCORE );
        free_alignment( alist );
    }

    gettimeofday( &finish, NULL );

    double elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_usec - start.tv_usec) / 1000000.0;

    return elapsed;
}

int main( int argc, char**argv ) {
    FILE *f = fopen( "results/31_03_2015_pairwise", "w" );
    if( f == NULL ) {
        printf( "Error opening file!\n" );
        exit( 1 );
    }

    int SIMD[2] = { COMPUTE_ON_SSE41, COMPUTE_ON_AVX2 };
    int bit_width[2] = { 8, 16 };

    size_t hit_count = 10;

    int iterations = 10;
    int internal_iterations = 10000;

    set_output_mode( OUTPUT_SILENT );

    init_score_matrix( MATRIX_BUILDIN, BLOSUM50 );
    init_gap_penalties( -3, -1 );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );

    set_threads( 1 );
    init_db_fasta( "data/P18080.fasta" );

    p_query query = init_sequence_fasta( READ_FROM_FILE, "data/Q3ZAI3.fasta" );

    for( int type = 0; type < 2; ++type ) {
        p_alignment_list (*align_func)( p_query, size_t, int, int );
        if( type == SW ) {
            align_func = &sw_align;
        }
        else {
            align_func = &nw_align;
        }

        for( int s = 0; s < 2; ++s ) {
            set_simd_compute_mode( SIMD[s] );

            for( int b = 0; b < 2; ++b ) {
                fprintf( f, "%s,%s,%d_bit", SIMD_DESC( SIMD[s] ), TYPE_DESC( type ), bit_width[b] );
                printf( "%s,%s,%d_bit", SIMD_DESC( SIMD[s] ), TYPE_DESC( type ), bit_width[b] );

                for( int i = 0; i < iterations; i++ ) {
                    double time = run_alignment( align_func, query, hit_count, bit_width[b], internal_iterations );

                    fprintf( f, ",%lf", time );
                    printf( ",%lf", time );
                }
                fprintf( f, "\n" );
                printf( "\n" );
            }
        }

        fprintf( f, "NO_SIMD,%s,64_bit", TYPE_DESC( type ) );
        printf( "NO_SIMD,%s,64_bit", TYPE_DESC( type ) );

        for( int i = 0; i < iterations; i++ ) {
            double time = run_alignment( align_func, query, hit_count, 64, internal_iterations );

            fprintf( f, ",%lf", time );
            printf( ",%lf", time );
        }
        fprintf( f, "\n" );
        printf( "\n" );
    }

    free_sequence( query );
    ssa_exit();

    fclose( f );

    return 0;
}
