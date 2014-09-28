/*
 * main.c
 *
 *  Created on: 22 Mar 2014
 *      Author: kaos
 */
#include "swalign.h"

int main() {
    char* d = "TACGGGTAT";
    char* q = "GGACGTACG";

    seq_pair problem;
    seq_pair_t result;

    problem.a = q;
    problem.alen = strlen( problem.a );
    problem.b = d;
    problem.blen = strlen( problem.b );

    result = smith_waterman( &problem, false );

    printf( "%s\n%s\n", result->a, result->b );

    return 0;
}

