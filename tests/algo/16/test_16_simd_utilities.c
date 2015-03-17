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

#include "../../tests.h"

#include "../../../src/cpu_config.h"
#include "../../../src/matrices.h"
#include "../../../src/query.h"
#include "../../../src/algo/searcher.h"
#include "../../../src/algo/16/search_16.h"
#include "../../../src/algo/16/search_16_util.h"
#include "../../../src/db_adapter.h"
#include "../../../src/util/debug_tools.h"
#include "../../../src/util/util_sequence.h"

#define MATCH 111
#define MISMATCH -40

static p_s16info setup_simd_util_test( char * query_string ) {
    mat_init_constant_scoring( MATCH, MISMATCH );
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    p_query query = query_read_from_string( "short query", query_string );

    p_search_data sdp = s_create_searchdata( query );

    return search_16_init( sdp );
}

static void exit_simd_util_test( p_s16info s ) {
    search_16_exit( s );
    mat_free();

    reset_compute_capability();
}

static void test_dprofile_16( int16_t * dprofile, int channels, sequence_t dseq ) {
    for( int i = 0; i < SCORE_MATRIX_DIM; ++i ) {
        for( int j = 0; j < CDEPTH_16_BIT; ++j ) {
            for( int k = 0; k < channels; k++ ) {
                int16_t val = dprofile[channels * CDEPTH_16_BIT * i + channels * j + k];

                if( k == 0 && i != 0 ) {
                    if( i == dseq.seq[j] ) {
                        ck_assert_int_eq( MATCH, val );
                    }
                    else {
                        ck_assert_int_eq( MISMATCH, val );
                    }
                }
                else {
                    ck_assert_int_eq( -1, val );
                }
            }
        }
    }
}

START_TEST (test_sse_simple)
    {
        set_max_compute_capability( COMPUTE_ON_SSE2 );

        p_s16info s = setup_simd_util_test( "AT" );

        uint8_t dseq_search_window[CDEPTH_16_BIT * CHANNELS_16_BIT_SSE];
        memset( dseq_search_window, 0, sizeof(uint8_t) * CDEPTH_16_BIT * CHANNELS_16_BIT_SSE );

        sequence_t dseq = us_prepare_sequence( "AATG", 4, 0, 0 );

        for( int i = 0; i < CDEPTH_16_BIT; ++i ) {
            dseq_search_window[i * CHANNELS_16_BIT_SSE] = dseq.seq[i];
        }

        dprofile_fill_16_sse2( s->dprofile, dseq_search_window );

        test_dprofile_16( (int16_t*) s->dprofile, CHANNELS_16_BIT_SSE, dseq );

        exit_simd_util_test( s );
    }END_TEST

START_TEST (test_avx_simple)
    {
        set_max_compute_capability( COMPUTE_ON_AVX2 );

        p_s16info s = setup_simd_util_test( "AT" );

        uint8_t dseq_search_window[CDEPTH_16_BIT * CHANNELS_16_BIT_AVX];
        memset( dseq_search_window, 0, sizeof(uint8_t) * CDEPTH_16_BIT * CHANNELS_16_BIT_AVX );

        sequence_t dseq = us_prepare_sequence( "AATG", 4, 0, 0 );

        for( int i = 0; i < CDEPTH_16_BIT; ++i ) {
            dseq_search_window[i * CHANNELS_16_BIT_AVX] = dseq.seq[i];
        }
        dprofile_fill_16_avx2( s->dprofile, dseq_search_window );

        test_dprofile_16( (int16_t*) s->dprofile, CHANNELS_16_BIT_AVX, dseq );

        exit_simd_util_test( s );
    }END_TEST

void add_16_simd_utilities_TC( Suite *s ) {
    TCase *tc_core = tcase_create( "16 bit SIMD utilities" );
    tcase_add_test( tc_core, test_sse_simple );
    tcase_add_test( tc_core, test_avx_simple );

    suite_add_tcase( s, tc_core );
}
