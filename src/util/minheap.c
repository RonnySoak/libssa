/*
 Copyright (C) 2014 Torbjorn Rognes
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU Affero General Public License for more details.
 You should have received a copy of the GNU Affero General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.

 Contact: Torbjorn Rognes <torognes@ifi.uio.no>,
 Department of Informatics, University of Oslo,
 PO Box 1080 Blindern, NO-0316 Oslo, Norway
 */

#include "minheap.h"

#include <stdlib.h>
#include <stdio.h>

#include "../util.h"

/* implement a priority queue with a min heap binary array structure */
/* elements with the lowest count should be at the top (root) */

/* return 1 if a is smaller than b, 0 if equal or greater */
static inline int elem_smaller(elem_t * a, elem_t * b) {
    if (a->score < b->score)
        return 1;
    return 0;
}

p_minheap minheap_init(int size) {
    p_minheap m = xmalloc(sizeof(struct minheap_s));
    m->alloc = size;
    m->array = xmalloc(size * sizeof(elem_t));
    m->count = 0;
    return m;
}

void minheap_exit(p_minheap m) {
    free(m->array);
    free(m);
}

static void minheap_replaceroot(p_minheap m, elem_t tmp) {
    /* remove the element at the root, then swap children up
     to the root and insert tmp at suitable place */

    /* start with root */
    int p = 0;
    int c = 2 * p + 1;
    /* while at least one child */
    while (c < m->count) {
        /* if two children: swap with the one with smallest value */
        if ((c + 1 < m->count)
                && (elem_smaller(m->array + c + 1, m->array + c)))
            c++;
        /* swap parent and child if child has lower value */
        if (elem_smaller(m->array + c, &tmp)) {
            m->array[p] = m->array[c];
        }
        else
            break;
        /* step down */
        p = c;
        c = 2 * p + 1;
    }
    m->array[p] = tmp;
}

void minheap_add(p_minheap m, elem_t * n) {
    if (m->count < m->alloc) {
        /* space for another item at end; swap upwards */
        int i = m->count++;
        int p = (i - 1) / 2;
        while ((i > 0) && elem_smaller(n, m->array + p)) {
            m->array[i] = m->array[p];
            i = p;
            p = (i - 1) / 2;
        }
        m->array[i] = *n;
    }
    else if (elem_smaller(m->array, n)) {
        /* replace the root if new element is larger than root */
        minheap_replaceroot(m, *n);
    }
}

#if 0
inline int
minheap_isempty(p_minheap m)
{
    return !m->count;
}
inline void
minheap_empty(p_minheap m)
{
    m->count = 0;
}
#endif

//elem_t minheap_pop(p_minheap m) {
//    /* return top element and restore order */
//    static elem_t zero = { 0, 0, 0 };
//    if (m->count) {
//        elem_t top = m->array[0];
//        m->count--;
//        if (m->count) {
//            elem_t tmp = m->array[m->count];
//            minheap_replaceroot(m, tmp);
//        }
//        return top;
//    }
//    else
//        return zero;
//}

static int minheap_compare(const void * a, const void * b) {
    elem_t * x = (elem_t*) a;
    elem_t * y = (elem_t*) b;

//
//    seq_buffer qx = *(seq_buffer*)x->query;
//
//    printf("in search\n");
//    printf("M: %ld\n", qx.seq.len);
//    printf("Mp: %p\n", &qx.seq.len);
//    printf("ap: %p\n", qx.seq.seq);
//    printf("a: %d%d%d\n", qx.seq.seq[0], qx.seq.seq[1], qx.seq.seq[2]);
//
//    seq_buffer qy = *(seq_buffer*)y->query;
//
//    printf("in search\n");
//    printf("M: %ld\n", qy.seq.len);
//    printf("Mp: %p\n", &qy.seq.len);
//    printf("ap: %p\n", qy.seq.seq);
//    printf("a: %d%d%d\n", qy.seq.seq[0], qy.seq.seq[1], qy.seq.seq[2]);

    if (x->score < y->score)
        return +1;
    else if (x->score > y->score)
        return -1;
    return 0;
}

void minheap_sort(p_minheap m) {
    qsort(m->array, m->count, sizeof(elem_t), minheap_compare);
}

void minheap_dump(p_minheap m) {
    for (int i = 0; i < m->count; i++)
        printf("%s%ld", i > 0 ? " " : "", m->array[i].score);
    printf("\n");
}
//
//elem_t minheap_poplast(p_minheap m) {
//    /* return top element and restore order */
//    static elem_t zero = { 0, 0, 0 };
//    if (m->count)
//        return m->array[--m->count];
//    else
//        return zero;
//}
//
//void minheap_test() {
//    p_minheap m = minheap_init(10000000);
//    int samples = 10000000;
//    swaps = 0;
//    for (int i = samples; i >= 0; i--) {
//        elem_t x = { random(), 0, 1 };
//        minheap_add(m, &x);
//    }
//    minheap_sort(m);
//    while (!minheap_isempty(m)) {
//        elem_t x = minheap_poplast(m);
//        printf("%d\n", x.count);
//    }
//    printf("Swaps: %d\n\n", swaps);
//    minheap_exit(m);
//}
