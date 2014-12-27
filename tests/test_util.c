/*
 * test_util.c
 *
 *  Created on: Oct 2, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "tests.h"

#include <stdint.h>
#include <stdio.h>

#include "../src/util/util.h"
#include "../src/libssa_extern_db.h"

/**
 * A small helper, to print sequences in decimal representation.
 */
void print_sequence(char* desc, int8_t* seq, long len) {
    printf("seq (%s): '", desc);
    for (int i = 0; i < len; i++) {
        printf("%d", seq[i]);
    }
    printf("'\n");
}

void print_sequence2(char* desc, sequence seq) {
    print_sequence(desc, (int8_t *)seq.seq, seq.len);
}

/**
 * Prints the addresses of a list of variables.
 */
void dbg_print(int n, void* param) {
    int i;
    for (i = 0; i < n; i++) {
        printf("Param %d: %p\n", i, (param + i));
    }
}

START_TEST (test_xmalloc)
    {
        uint8_t * arr = xmalloc(5);
        for (int i = 0; i < 5; ++i) {
            arr[i] = 1;
        }

        p_seqinfo * arr2 = xmalloc(5 * sizeof(p_seqinfo));
        for (int i = 0; i < 5; ++i) {
            arr2[i] = xmalloc(sizeof(seqinfo));
            arr2[i]->ID = i;
        }

        for (int i = 0; i < 5; ++i) {
            free(arr2[i]);
        }
        free(arr2);
        free(arr);
    }END_TEST


START_TEST (test_xrealloc)
    {
        p_seqinfo * arr = xmalloc(5 * sizeof(p_seqinfo));
        for (int i = 0; i < 5; ++i) {
            arr[i] = xmalloc(sizeof(seqinfo));
            arr[i]->ID = i;
        }

        arr = xrealloc(arr, 10 * sizeof(p_seqinfo));
        for (int i = 0; i < 10; ++i) {
            if(i >= 5) {
                arr[i] = xmalloc(sizeof(seqinfo));
                arr[i]->ID = i;
            }
        }

        for (int i = 0; i < 10; ++i) {
            free(arr[i]);
        }
        free(arr);
    }END_TEST

void addUtilTC(Suite *s) {
    TCase *tc_core = tcase_create("util");
    tcase_add_test(tc_core, test_xmalloc);
    tcase_add_test(tc_core, test_xrealloc);

    suite_add_tcase(s, tc_core);
}
