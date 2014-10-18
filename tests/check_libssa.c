/*
 * check_libssa.c
 *
 *  Created on: 27 Jun 2014
 *      Author: kaos
 */

#include "tests.h"

#include "../src/libssa.h"

Suite* libssa_suite(void) {
    Suite *s = suite_create("libssa");

    /* Core test case */
    addMinHeapTC(s);
    addUtilSequenceTC(s);
    addMatricesTC(s);
    addQueryTC(s);
    addDBIteratorTC(s);
    addCigarTC(s);
    addAlignTC(s);
    addSmithWaterman63TC(s);
    addSearcherTC(s);
    addAlignerTC(s);
    addManagerTC(s);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s = libssa_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
