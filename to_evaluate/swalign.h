/*
 *  Copyright (c) 2010 Nicolaus Lance Hepler
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 */

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GAP -1.0
#define MATCH 2.0
#define MISMATCH -0.5

typedef enum {
    true, false
} bool;

typedef struct {
    char *a;
    unsigned int alen;
    char *b;
    unsigned int blen;
} seq_pair;
typedef seq_pair *seq_pair_t;

typedef struct {
    double score;
    unsigned int prev[2];
} entry;
typedef entry *entry_t;

typedef struct {
    unsigned int m;
    unsigned int n;
    entry_t **mat;
} matrix;
typedef matrix *matrix_t;

char* reverse( char *str );

seq_pair_t traceback( seq_pair_t problem, matrix_t S, bool local );

matrix_t create_matrix( unsigned int m, unsigned int n );

void destroy_matrix( matrix_t S );

void destroy_seq_pair( seq_pair_t pair );

seq_pair_t smith_waterman( seq_pair_t problem, bool local );
