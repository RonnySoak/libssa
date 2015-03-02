/*
 * check_libssa.c
 *
 *  Created on: 27 Jun 2014
 *      Author: kaos
 */

#include "tests.h"

#include "../src/libssa.h"

Suite* libssa_suite( void ) {
    Suite *s = suite_create( "libssa" );

    set_threads( 1 );

    /* Core test case */
    addCPUConfigTC( s );
    addUtilTC( s );
    addMinHeapTC( s );
    addLinkedListTC( s );
    addThreadPoolTC( s );
    addUtilSequenceTC( s );
    addMatricesTC( s );
    addQueryTC( s );
    addDBIteratorTC( s );
    addCigarTC( s );
    addAlignTC( s );
    add_sw_63_TC( s );
    add_16_simd_utilities_TC( s );
    add_8_simd_utilities_TC( s );
    add_sw_16_SSE2_TC( s );
    add_nw_16_SSE2_TC( s );
    add_sw_16_AVX2_TC( s );
    add_nw_16_AVX2_TC( s );
    add_sw_8_SSE41_TC( s );
    add_nw_8_SSE41_TC( s );
    add_sw_8_AVX2_TC( s );
    add_nw_8_AVX2_TC( s );
    addSearcher63TC( s );
    addSearcherTC( s );
    addAlignerTC( s );
    addManagerTC( s );
    addLibssaTC( s );
    addBiggerDatabasesTC( s );

    return s;
}

int main( void ) {
    int number_failed;
    Suite *s = libssa_suite();
    SRunner *sr = srunner_create( s );
    srunner_run_all( sr, CK_NORMAL );
    number_failed = srunner_ntests_failed( sr );
    srunner_free( sr );
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
