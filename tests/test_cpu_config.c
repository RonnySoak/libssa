/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "tests.h"
#include "../src/cpu_config.h"

START_TEST (test_cpu_config)
    {
        test_cpu_features();
    }END_TEST

void addCPUConfigTC( Suite *s ) {
    TCase *tc_core = tcase_create( "cpu_config" );
    tcase_add_test( tc_core, test_cpu_config );

    suite_add_tcase( s, tc_core );
}

