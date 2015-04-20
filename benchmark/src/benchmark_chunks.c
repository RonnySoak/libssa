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

#define PROTEIN_E 0
#define NUCLEOTIDE_E 1

static p_query change_environment( int environment ) {
    char * db;
    char * query;
    if( environment == PROTEIN_E ) {
        db = "uniprot_sprot";
        query = "P19930";

        init_score_matrix( MATRIX_BUILDIN, BLOSUM50 );
    }
    else {
        db = "Rfam_11_0";
        query = "rfam_sequence";

        init_constant_scores( 5, -4 );
    }

    char * filename = concat( concat( "data/", db ), ".fasta" );
    init_db_fasta( filename );
    free( filename );

    return read_query( query );
}

int main( int argc, char**argv ) {
    FILE *f = open_log_file( "chunks" );

    int threads[2] = { 1, 8 };
    size_t chunk_size[2] = { 50000, 100000, }; // 10, 100, 500, 1000, 1500, 2500, 5000, 10000, 25000, 
    int SIMD[2] = { COMPUTE_ON_SSE41, COMPUTE_ON_AVX2 };
    int bit_width[2] = { 8, 16 };

    int environment[2] = { PROTEIN_E, NUCLEOTIDE_E };

    size_t hit_count = 10;

    int iterations = 10;

    set_output_mode( OUTPUT_SILENT );

    init_gap_penalties( -3, -1 );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );

    for( int t = 0; t < 2; ++t ) {
        set_thread_count( threads[t] );

        for( int type = 0; type < 2; ++type ) {
            for( int s = 0; s < 2; ++s ) {
                set_simd_compute_mode( SIMD[s] );

                for( int b = 0; b < 2; ++b ) {
                    for( int e = 0; e < 2; ++e ) {

                        p_query query = change_environment( environment[e] );

                        char * e_desc = (environment[e] == PROTEIN_E) ? "uniprot_sprot" : "Rfam_11_0";

                        for( int c = 0; c < 2; ++c ) {
                            set_chunk_size( chunk_size[c] );

                            log_to_file( f, "%s,%s,%s,%d_bit,%d_t,%ld", e_desc, SIMD_DESC( SIMD[s] ), TYPE_DESC( type ),
                                    bit_width[b], threads[t], chunk_size[c] );

                            run_and_log_n_times( f, type, query, hit_count, bit_width[b], iterations );
                        }

                        free_sequence( query );
                    }
                }
            }
        }
    }

    ssa_exit();

    fclose( f );

    return 0;
}
