/*
 * libssa_example.c
 *
 *  Created on: Sep 4, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../src/libssa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static char* concat( char * s1, char * s2 ) {
    char *result = malloc( strlen( s1 ) + strlen( s2 ) + 1 ); //+1 for the zero-terminator
    strcpy( result, s1 );
    strcat( result, s2 );
    return result;
}

/*
 * Using UniProtKB/Swiss-Prot database.
 *
 * Can be downloaded from here: http://www.uniprot.org/downloads
 */

#define SW 0
#define NW 1

#define SIMD_DESC(s) ( (s == COMPUTE_ON_AVX2) ? "AVX2" : "SSE41" )
#define TYPE_DESC(t) ( (t == SW) ? "SW" : "NW" )

static double run_alignment( p_alignment_list (*align_func)( p_query, size_t, int, int ), p_query query, size_t hit_count,
        int bit_width ) {

    struct timeval start;
    struct timeval finish;

    gettimeofday( &start, NULL );

    p_alignment_list alist = align_func( query, hit_count, bit_width, COMPUTE_SCORE );

    gettimeofday( &finish, NULL );

    free_alignment( alist );

    double elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_usec - start.tv_usec) / 1000000.0;

    return elapsed;
}

#define T_COUNT 6
#define B_COUNT 2
#define A_COUNT 2

int main( int argc, char**argv ) {
    FILE *f = fopen( "results/30_03_2015_threads", "w" );
    if( f == NULL ) {
        printf( "Error opening file!\n" );
        exit( 1 );
    }

    int threads[T_COUNT] = { 2, 3, 5, 6, 7, 10 };
    int bit_width[B_COUNT] = { 8, 16 };
    char * query = "P18080";

    size_t hit_count = 10;

    int iterations = 10;

    set_output_mode( OUTPUT_SILENT );

    init_score_matrix( MATRIX_BUILDIN, BLOSUM50 );
    init_gap_penalties( -3, -1 );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );
    init_db_fasta( "data/uniprot_sprot.fasta" );

    for( int t = 0; t < T_COUNT; ++t ) {
        set_threads( threads[t] );

        for( int type = 0; type < A_COUNT; ++type ) {
            p_alignment_list (*align_func)( p_query, size_t, int, int );
            if( type == SW ) {
                align_func = &sw_align;
            }
            else {
                align_func = &nw_align;
            }

            set_simd_compute_mode( COMPUTE_ON_AVX2 );

            for( int b = 0; b < B_COUNT; ++b ) {
                char * filename = concat( concat( "data/", query ), ".fasta" );
                p_query query_profile = init_sequence_fasta( READ_FROM_FILE, filename );
                free( filename );

                fprintf( f, "%s,%s,%s,%d_bit,%d_t", query, SIMD_DESC( COMPUTE_ON_AVX2 ), TYPE_DESC( type ),
                        bit_width[b], threads[t] );
                printf( "%s,%s,%s,%d_bit,%d_t", query, SIMD_DESC( COMPUTE_ON_AVX2 ), TYPE_DESC( type ),
                        bit_width[b], threads[t] );

                for( int i = 0; i < iterations; i++ ) {
                    double time = run_alignment( align_func, query_profile, hit_count, bit_width[b] );

                    fprintf( f, ",%lf", time );
                    printf( ",%lf", time );
                }
                fprintf( f, "\n" );
                printf( "\n" );

                free_sequence( query_profile );
            }

            // 64 bit
            char * filename = concat( concat( "data/", query ), ".fasta" );
            p_query query_profile = init_sequence_fasta( READ_FROM_FILE, filename );
            free( filename );

            fprintf( f, "%s,NO_SIMD,%s,%d_bit,%d_t", query, TYPE_DESC( type ), 64, threads[t] );
            printf( "%s,NO_SIMD,%s,%d_bit,%d_t", query, TYPE_DESC( type ), 64, threads[t] );

            for( int i = 0; i < iterations; i++ ) {
                double time = run_alignment( align_func, query_profile, hit_count, 64 );

                fprintf( f, ",%lf", time );
                printf( ",%lf", time );
            }
            fprintf( f, "\n" );
            printf( "\n" );

            free_sequence( query_profile );
        }
    }

    ssa_exit();

    fclose( f );

    return 0;
}
