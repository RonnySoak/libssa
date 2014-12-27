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

#include "util.h"

/* implement a priority queue with a min heap binary array structure */
/* elements with the lowest count should be at the top (root) */

/* return 1 if a is smaller than b, 0 if equal or greater */
static inline int elem_smaller(elem_t * a, elem_t * b) {
    if (a->score < b->score)
        return 1;
    return 0;
}

p_minheap minheap_init(int size) {
    p_minheap m = xmalloc(sizeof(struct minheap_t));
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

static int minheap_compare(const void * a, const void * b) {
    elem_t * x = (elem_t*) a;
    elem_t * y = (elem_t*) b;

    return CMP_ASC(x->score, y->score);
}

void minheap_sort(p_minheap m) {
    qsort(m->array, m->count, sizeof(elem_t), minheap_compare);
}
