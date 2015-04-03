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

#define S_COUNT 2
#define B_COUNT 2
#define Q_COUNT 36
#define A_COUNT 2

int main( int argc, char**argv ) {
    FILE *f = open_log_file( "queries" );

    int SIMD[S_COUNT] = { COMPUTE_ON_SSE41, COMPUTE_ON_AVX2 };
    int bit_width[B_COUNT] = { 8, 16 };
    char * queries[Q_COUNT] = { "Q9UKN1", "P33450", "Q7TMA5", "P08519", "P20930", "P0C6B8", "P28167", "P19096",
            "P04775", "Q8LLD0", "P27895", "P21177", "P42357", "P03435", "P25705", "P18080", "P58229", "P10635",
            "P01008", "Q3ZAI3", "P07327", "P03989", "Q8ZGB4", "P53765", "Q60341", "P00762", "P14942", "P19930",
            "P05013", "P01111", "P02232", "P03630", "O74807", "P07765", "O29181", "P56980", };

    size_t hit_count = 10;

    int iterations = 10;

    set_output_mode( OUTPUT_SILENT );

    init_score_matrix( MATRIX_BUILDIN, BLOSUM50 );
    init_gap_penalties( -3, -1 );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );
    init_db_fasta( "data/uniprot_sprot.fasta" );

    set_threads( 8 );

    for( int type = 0; type < A_COUNT; ++type ) {
        for( int q = (Q_COUNT - 1); q >= 0; --q ) {
            p_query query = read_query(  queries[q] );

            for( int s = 0; s < S_COUNT; ++s ) {
                set_simd_compute_mode( SIMD[s] );

                for( int b = 0; b < B_COUNT; ++b ) {

                    log_to_file( f, "%s,%s,%s,%d_bit", queries[q], SIMD_DESC( SIMD[s] ), TYPE_DESC( type ), bit_width[b] );

                    run_and_log_n_times( f, type, query, hit_count, bit_width[b], iterations );
                }
            }

            free_sequence( query );
        }
    }

    ssa_exit();

    fclose( f );

    return 0;
}
