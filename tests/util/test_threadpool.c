/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include <sys/sysinfo.h>

#include "../../src/util/thread_pool.h"
#include "../../src/libssa.h"
#include "../../src/util/util.h"

void setup_pool( int nr_of_threads ) {
    set_threads( nr_of_threads );

    init_thread_pool();
}

void teardown_pool() {
    exit_thread_pool();

    set_threads( 1 );
}

void * simple_test_thread( void * args ) {
    int * counter = xmalloc( sizeof(int) );

    for( int i = 0; i < 1000; ++i ) {
        (*counter)++;
    }

    return counter;
}

START_TEST (test_one_thread)
    {
        setup_pool( 1 );

        start_threads( &simple_test_thread );

        ck_assert_int_eq( 1, get_current_thread_count() );

        int * result_list[1];

        wait_for_threads( (void **) &result_list );

        ck_assert_int_eq( 1000, *result_list[0] );

        teardown_pool();
    }END_TEST

START_TEST (test_nr_of_cores_threads)
    {
        setup_pool( -1 );

        start_threads( &simple_test_thread );

        ck_assert_int_eq( get_nprocs(), get_current_thread_count() );

        int * result_list[get_current_thread_count()];

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < get_current_thread_count(); ++i ) {
            ck_assert_int_eq( 1000, *result_list[i] );
        }

        teardown_pool();
    }END_TEST

START_TEST (test_20_threads)
    {
        int n = 20;
        setup_pool( n );

        start_threads( &simple_test_thread );

        ck_assert_int_eq( n, get_current_thread_count() );

        int * result_list[n];

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < n; ++i ) {
            ck_assert_int_eq( 1000, *result_list[i] );
        }

        teardown_pool();
    }END_TEST

START_TEST (test_2000_threads)
    {
        int n = 2000;
        setup_pool( n );

        start_threads( &simple_test_thread );

        ck_assert_int_eq( n, get_current_thread_count() );

        int * result_list[n];

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < n; ++i ) {
            ck_assert_int_eq( 1000, *result_list[i] );
        }

        teardown_pool();
    }END_TEST

void addThreadPoolTC( Suite *s ) {
    TCase *tc_core = tcase_create( "threadpool" );
    tcase_add_test( tc_core, test_one_thread );
    tcase_add_test( tc_core, test_nr_of_cores_threads );
    tcase_add_test( tc_core, test_20_threads );
    tcase_add_test( tc_core, test_2000_threads );

    suite_add_tcase( s, tc_core );
}
