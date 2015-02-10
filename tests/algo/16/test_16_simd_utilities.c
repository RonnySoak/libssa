/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../../tests.h"

#include "../../../src/cpu_config.h"
#include "../../../src/matrices.h"
#include "../../../src/query.h"
#include "../../../src/db_iterator.h"
#include "../../../src/algo/searcher.h"
#include "../../../src/algo/16/search_16.h"
#include "../../../src/algo/16/search_16_util.h"
#include "../../../src/util/debug_tools.h"

#define MATCH 111
#define MISMATCH -40

static p_s16info setup_simd_util_test( char * query_string, char * db_file, int hit_count ) {
    mat_init_constant_scoring( MATCH, MISMATCH );
    init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

    p_query query = query_read_from_string( "short query", query_string );

    ssa_db_init_fasta( concat( "./tests/testdata/", db_file ) );

    p_search_data sdp = s_create_searchdata( query, hit_count );

    return search_16_init( sdp );
}

static void exit_simd_util_test( p_s16info s ) {
    search_16_exit( s );
    it_free();
    mat_free();
    free_db();

    reset_compute_capability();
}

static void test_dprofile_16( int16_t * dprofile, int channels, sequence dseq ) {
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

        p_s16info s = setup_simd_util_test( "AT", "short_db.fas", 1 );

        uint8_t dseq_search_window[CDEPTH_16_BIT * CHANNELS_16_BIT_SSE];
        memset( dseq_search_window, 0, CDEPTH_16_BIT * CHANNELS_16_BIT_SSE );

        sequence dseq = it_translate_sequence( it_get_sequence( 0 ), 0, 0 );

        for( int i = 0; i < CDEPTH_16_BIT; ++i ) {
            dseq_search_window[i * CHANNELS_16_BIT_SSE] = dseq.seq[i];
        }

        int16_t * dprofile = (int16_t*) s->dprofile_sse;

        dprofile_fill_16_sse2( dprofile, dseq_search_window );

        test_dprofile_16( dprofile, CHANNELS_16_BIT_SSE, dseq );

        exit_simd_util_test( s );
    }END_TEST

START_TEST (test_avx_simple)
    {
        set_max_compute_capability( COMPUTE_ON_AVX2 );

        p_s16info s = setup_simd_util_test( "AT", "short_db.fas", 1 );

        uint8_t dseq_search_window[CDEPTH_16_BIT * CHANNELS_16_BIT_AVX];
        memset( dseq_search_window, 0, CDEPTH_16_BIT * CHANNELS_16_BIT_AVX );

        sequence dseq = it_translate_sequence( it_get_sequence( 0 ), 0, 0 );

        for( int i = 0; i < CDEPTH_16_BIT; ++i ) {
            dseq_search_window[i * CHANNELS_16_BIT_AVX] = dseq.seq[i];
        }
        int16_t * dprofile = (int16_t*) s->dprofile_avx;

        dprofile_fill_16_avx2( dprofile, dseq_search_window );

        test_dprofile_16( dprofile, CHANNELS_16_BIT_AVX, dseq );

        exit_simd_util_test( s );
    }END_TEST

void add_16_simd_utilities_TC( Suite *s ) {
    TCase *tc_core = tcase_create( "16 bit SIMD utilities" );
    tcase_add_test( tc_core, test_sse_simple );
    tcase_add_test( tc_core, test_avx_simple );

    suite_add_tcase( s, tc_core );
}
