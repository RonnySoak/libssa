/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/util/util.h"
#include "../../src/util/minheap.h"

static elem_t new_elem( long value, size_t db_id, int query_id ) {
    elem_t e;
    e.db_id = db_id;
    e.query_id = query_id;
    e.score = value;
    return e;
}

START_TEST (test_add_size_elements)
    {
        p_minheap m = minheap_init( 5 );

        ck_assert_int_eq( 0, m->count );
        ck_assert_int_eq( 5, m->alloc );

        elem_t e1 = new_elem( 3, 11, 1 );
        minheap_add( m, &e1 );
        elem_t e2 = new_elem( 4, 22, 2 );
        minheap_add( m, &e2 );
        elem_t e3 = new_elem( 2, 33, 3 );
        minheap_add( m, &e3 );
        elem_t e4 = new_elem( 5, 44, 4 );
        minheap_add( m, &e4 );
        elem_t e5 = new_elem( 1, 55, 5 );
        minheap_add( m, &e5 );

        minheap_sort( m );

        ck_assert_int_eq( 5, m->count );
        ck_assert_int_eq( 5, m->alloc );

        ck_assert_int_eq( 5, m->array[0].score );
        ck_assert_int_eq( 4, m->array[0].query_id );
        ck_assert_int_eq( 44, m->array[0].db_id );

        minheap_exit( m );
    }END_TEST

START_TEST (test_add_more)
    {
        p_minheap m = minheap_init( 5 );

        ck_assert_int_eq( 0, m->count );
        ck_assert_int_eq( 5, m->alloc );

        for( int i = 0; i < 20; i++ ) {
            elem_t e = new_elem( i, i, i );
            minheap_add( m, &e );
        }

        minheap_sort( m );

        ck_assert_int_eq( 5, m->count );
        ck_assert_int_eq( 5, m->alloc );

        ck_assert_int_eq( 19, m->array[0].score );
        ck_assert_int_eq( 19, m->array[0].query_id );
        ck_assert_int_eq( 19, m->array[0].db_id );

        ck_assert_int_eq( 18, m->array[1].score );
        ck_assert_int_eq( 17, m->array[2].score );
        ck_assert_int_eq( 16, m->array[3].score );
        ck_assert_int_eq( 15, m->array[4].score );

        minheap_exit( m );
    }END_TEST

void addMinHeapTC( Suite *s ) {
    TCase *tc_core = tcase_create( "minheap" );
    tcase_add_test( tc_core, test_add_size_elements );
    tcase_add_test( tc_core, test_add_more );
    suite_add_tcase( s, tc_core );
}

