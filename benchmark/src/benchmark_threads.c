/*
 * libssa_example.c
 *
 *  Created on: Sep 4, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../../src/libssa.h"

#include <stdio.h>
#include <stdlib.h>

#include "benchmark_util.h"

#define T_COUNT 6
#define B_COUNT 2
#define A_COUNT 2

int main( int argc, char**argv ) {
    FILE *f = open_log_file( "threads" );

    int threads[T_COUNT] = { 2, 3, 5, 6 };
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
            set_simd_compute_mode( COMPUTE_ON_AVX2 );

            for( int b = 0; b < B_COUNT; ++b ) {
                p_query query_profile = read_query(  query );

                log_to_file( f, "%s,%s,%s,%d_bit,%d_t", query, SIMD_DESC( COMPUTE_ON_AVX2 ), TYPE_DESC( type ),
                        bit_width[b], threads[t] );

                run_and_log_n_times( f, type, query_profile, hit_count, bit_width[b], iterations );

                free_sequence( query_profile );
            }

            // 64 bit
            p_query query_profile = read_query(  query );

            log_to_file( f, "%s,NO_SIMD,%s,%d_bit,%d_t", query, TYPE_DESC( type ), 64, threads[t] );

            run_and_log_n_times( f, type, query_profile, hit_count, 64, iterations );

            free_sequence( query_profile );
        }
    }

    ssa_exit();

    fclose( f );

    return 0;
}
