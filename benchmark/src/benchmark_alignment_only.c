/*
 * libssa_example.c
 *
 *  Created on: Sep 4, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "../../src/libssa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "benchmark_util.h"

#define T_COUNT 1
#define S_COUNT 2
#define B_COUNT 2
#define Q_COUNT 4
#define A_COUNT 2

int main( int argc, char**argv ) {
    FILE *f = open_log_file( "alignment_only" );

    int threads[T_COUNT] = { 1, 4 };
    int SIMD[S_COUNT] = { COMPUTE_ON_SSE41, COMPUTE_ON_AVX2 };
    int bit_width[B_COUNT] = { 8, 16 };
    char * queries[Q_COUNT] = { "O74807", "P19930", "Q3ZAI3", "P18080" };

    size_t hit_count = 10;

    int iterations = 10;

    set_output_mode( OUTPUT_SILENT );

    init_score_matrix( MATRIX_BUILDIN, BLOSUM50 );
    init_gap_penalties( -3, -1 );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );
    init_db_fasta( "data/uniprot_sprot.fasta" );

    for( int t = 0; t < T_COUNT; ++t ) {
        set_thread_count( threads[t] );

        for( int type = 0; type < A_COUNT; ++type ) {
            for( int s = 0; s < S_COUNT; ++s ) {
                set_simd_compute_mode( SIMD[s] );

                for( int b = 0; b < B_COUNT; ++b ) {
                    for( int q = 0; q < Q_COUNT; ++q ) {
                        p_query query = read_query(  queries[q] );

                        log_to_file( f, "%s,%s,%s,%d_bit,%d_t", queries[q], SIMD_DESC( SIMD[s] ), TYPE_DESC( type ),
                                bit_width[b], threads[t] );

                        run_and_log_n_times( f, type, query, hit_count, bit_width[b], iterations );

                        free_sequence( query );
                    }
                }
            }

            // 64 bit
            for( int q = 0; q < Q_COUNT; ++q ) {
                p_query query = read_query(  queries[q] );

                log_to_file( f, "%s,NO_SIMD,%s,%d_bit,%d_t", queries[q], TYPE_DESC( type ), 64, threads[t] );

                run_and_log_n_times( f, type, query, hit_count, 64, iterations );

                free_sequence( query );
            }
        }
    }

    ssa_exit();

    fclose( f );

    return 0;
}
