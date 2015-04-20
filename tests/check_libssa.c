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

#include "tests.h"

#include "../src/libssa.h"

Suite* libssa_suite( void ) {
    Suite *s = suite_create( "libssa" );

    /*
     * All test should run in one thread, except some threading test cases.
     */
    set_thread_count( 1 );
    set_output_mode( OUTPUT_WARNING );

    addCPUConfigTC( s );
    addUtilTC( s );
    addMinHeapTC( s );
    addThreadPoolTC( s );
    addUtilSequenceTC( s );
    addMatricesTC( s );
    addQueryTC( s );
    addLibSSAExternDBTC( s );
    addDBAdapterTC( s );
    addCigarTC( s );
    addAlignTC( s );
    add_sw_64_TC( s );
    addSearcher64TC( s );
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
    addSearcherTC( s );
    addAlignerTC( s );
    addManagerTC( s );
    addLibssaTC( s );
    addBiggerDatabasesTC( s );

    return s;
}

int main( void ) {
    printf( "Using Check unit testing framework version %d.%d.%d\n", CHECK_MAJOR_VERSION, CHECK_MINOR_VERSION,
            CHECK_MICRO_VERSION );

    int number_failed;
    Suite *s = libssa_suite();
    SRunner *sr = srunner_create( s );
    srunner_run_all( sr, CK_NORMAL );
    number_failed = srunner_ntests_failed( sr );
    srunner_free( sr );
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
