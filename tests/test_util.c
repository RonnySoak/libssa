/*
 * test_util.c
 *
 *  Created on: Oct 2, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "stdint.h"
#include "stdio.h"

#include "tests.h"

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
