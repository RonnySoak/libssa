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

#include "../../src/libssa.h"

#include <stdio.h>
#include <stdlib.h>

#include "benchmark_util.h"

int main( int argc, char**argv ) {
    FILE *f = open_log_file( "pairwise" );

    int SIMD[2] = { COMPUTE_ON_SSE41, COMPUTE_ON_AVX2 };
    int bit_width[2] = { 8, 16 };

    size_t hit_count = 10;

    int iterations = 10;
    int internal_iterations = 10000;

    set_output_mode( OUTPUT_SILENT );

    init_score_matrix( MATRIX_BUILDIN, BLOSUM50 );
    init_gap_penalties( -3, -1 );
    init_symbol_translation( AMINOACID, FORWARD_STRAND, 3, 3 );

    set_thread_count( 1 );
    init_db( "data/P18080.fasta" );

    p_query query = init_sequence_fasta( READ_FROM_FILE, "data/Q3ZAI3.fasta" );

    for( int type = 0; type < 2; ++type ) {
        for( int s = 0; s < 2; ++s ) {
            set_simd_compute_mode( SIMD[s] );

            for( int b = 0; b < 2; ++b ) {
                log_to_file( f, "%s,%s,%d_bit", SIMD_DESC( SIMD[s] ), TYPE_DESC( type ), bit_width[b] );

                for( int i = 0; i < iterations; i++ ) {
                    double time = run_alignment_times( type, query, hit_count, bit_width[b], internal_iterations );

                    log_to_file( f, ",%lf", time );
                }
                log_to_file( f, "\n" );
            }
        }

        log_to_file( f, "NO_SIMD,%s,64_bit", TYPE_DESC( type ) );

        for( int i = 0; i < iterations; i++ ) {
            double time = run_alignment_times( type, query, hit_count, 64, internal_iterations );

            log_to_file( f, ",%lf", time );
        }
        log_to_file( f, "\n" );
    }

    free_sequence( query );
    ssa_exit();

    fclose( f );

    return 0;
}
