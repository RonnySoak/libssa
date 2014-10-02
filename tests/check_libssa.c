/*
 * check_libssa.c
 *
 *  Created on: 27 Jun 2014
 *      Author: kaos
 */

#include "tests.h"

#include "../src/libssa.h"

// TODO use it or delete it
void dbg_print(int n, void* param) {
    int i;
    for (i = 0; i < n; i++) {
        printf("Param %d: %p\n", i, (param + i));
    }
}

Suite* libssa_suite(void) {
    Suite *s = suite_create("libssa");

    /* Core test case */
    addSearcherTC(s);
    addSearch63TC(s);
    addQueryTC(s);
    addMatricesTC(s);
    addUtilSequenceTC(s);
    addDBIteratorTC(s);
    addMinHeapTC(s);

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
