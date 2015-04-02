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

static double run_alignment( p_alignment_list (*align_func)( p_query, size_t, int, int ), p_query query,
        size_t hit_count, int bit_width ) {

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

int main( int argc, char**argv ) {
    FILE *f = fopen( "results/31_03_2015_chunks", "w" );
    if( f == NULL ) {
        printf( "Error opening file!\n" );
        exit( 1 );
    }

    int threads[2] = { 1, 4 };
    size_t chunk_size[9] = { 10, 100, 500, 1000, 1500, 2500, 5000, 10000, 25000 };
    int SIMD[2] = { COMPUTE_ON_SSE41, COMPUTE_ON_AVX2 };
    int bit_width[2] = { 8, 16 };
    char * queries[2] = { "P19930", "Q3ZAI3" };
    char * dbs[2] = { "uniprot_sprot", "Rfam_11_0" };

    size_t hit_count = 10;

    int iterations = 10;

    set_output_mode( OUTPUT_SILENT );

    init_score_matrix( MATRIX_BUILDIN, BLOSUM50 );
    init_gap_penalties( -3, -1 );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );

    for( int t = 0; t < 3; ++t ) {
        set_threads( threads[t] );

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
                    for( int q = 0; q < 4; ++q ) {
                        for( int d = 0; d < 2; ++d ) {
                            char * filename = concat( concat( "data/", dbs[d] ), ".fasta" );
                            init_db_fasta( filename );
                            free( filename );

                            for( int c = 0; c < 9; ++c ) {
                                set_chunk_size( chunk_size[c] );

                                char * filename = concat( concat( "data/", queries[q] ), ".fasta" );
                                p_query query = init_sequence_fasta( READ_FROM_FILE, filename );
                                free( filename );

                                fprintf( f, "%s,%s,%s,%s,%d_bit,%d_t,%ld", dbs[d], queries[q], SIMD_DESC( SIMD[s] ),
                                        TYPE_DESC( type ), bit_width[b], threads[t], chunk_size[c] );
                                printf( "%s,%s,%s,%s,%d_bit,%d_t,%ld", dbs[d], queries[q], SIMD_DESC( SIMD[s] ),
                                        TYPE_DESC( type ), bit_width[b], threads[t], chunk_size[c] );

                                for( int i = 0; i < iterations; i++ ) {
                                    double time = run_alignment( align_func, query, hit_count, bit_width[b] );

                                    fprintf( f, ",%lf", time );
                                    printf( ",%lf", time );
                                }
                                fprintf( f, "\n" );
                                printf( "\n" );

                                free_sequence( query );
                            }
                        }
                    }
                }
            }
        }
    }

    ssa_exit();

    fclose( f );

    return 0;
}
