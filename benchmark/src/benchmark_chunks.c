/*
 * chunk_benchmark.c
 *
 *  Created on: Feb 25, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "../../src/libssa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "benchmark_util.h"

int main( int argc, char**argv ) {
    FILE *f = open_log_file( "chunks" );

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

    for( int t = 0; t < 2; ++t ) {
        set_threads( threads[t] );

        for( int type = 0; type < 2; ++type ) {
            for( int s = 0; s < 2; ++s ) {
                set_simd_compute_mode( SIMD[s] );

                for( int b = 0; b < 2; ++b ) {
                    for( int q = 0; q < 2; ++q ) {
                        for( int d = 0; d < 2; ++d ) {
                            char * filename = concat( concat( "data/", dbs[d] ), ".fasta" );
                            init_db_fasta( filename );
                            free( filename );

                            for( int c = 0; c < 9; ++c ) {
                                set_chunk_size( chunk_size[c] );

                                p_query query = read_query(  queries[q] );

                                log_to_file( f, "%s,%s,%s,%s,%d_bit,%d_t,%ld", dbs[d], queries[q], SIMD_DESC( SIMD[s] ),
                                        TYPE_DESC( type ), bit_width[b], threads[t], chunk_size[c] );

                                run_and_log_n_times( f, type, query, hit_count, bit_width[b], iterations );

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
