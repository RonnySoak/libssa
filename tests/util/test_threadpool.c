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

static void setup_pool( int nr_of_threads ) {
    set_threads( nr_of_threads );

    init_thread_pool();
}

static void teardown_pool() {
    exit_thread_pool();

    set_threads( 1 );
}

struct int_result {
    int val;
};

static void * simple_test_thread( void * size_arg ) {
    struct int_result * counter = xmalloc( sizeof(struct int_result) );

    for( int i = 0; i < *((size_t*) size_arg); ++i ) {
        counter->val++;
    }

    return counter;
}

START_TEST (test_one_thread)
    {
        setup_pool( 1 );

        size_t count = 1000;
        start_threads( &simple_test_thread, &count );

        ck_assert_int_eq( 1, get_current_thread_count() );

        int * result_list[1];

        wait_for_threads( (void **) &result_list );

        ck_assert_int_eq( count, *result_list[0] );

        teardown_pool();
    }END_TEST

START_TEST (test_nr_of_cores_threads)
    {
        setup_pool( -1 );

        size_t count = 1000;
        start_threads( &simple_test_thread, &count );

        ck_assert_int_eq( get_nprocs(), get_current_thread_count() );

        int * result_list[get_current_thread_count()];

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < get_current_thread_count(); ++i ) {
            ck_assert_int_eq( count, *result_list[i] );
        }

        teardown_pool();
    }END_TEST

START_TEST (test_20_threads)
    {
        int n = 20;
        setup_pool( n );

        size_t count = 1000;
        start_threads( &simple_test_thread, &count );

        ck_assert_int_eq( n, get_current_thread_count() );

        int * result_list[n];

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < n; ++i ) {
            ck_assert_int_eq( count, *result_list[i] );
        }

        teardown_pool();
    }END_TEST

START_TEST (test_1000_threads)
    {
        int n = 1000;
        setup_pool( n );

        size_t count = 1000;
        start_threads( &simple_test_thread, &count );

        ck_assert_int_eq( n, get_current_thread_count() );

        struct int_result * result_list[n];

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < n; ++i ) {
            ck_assert_int_eq( count, result_list[i]->val );
        }

        teardown_pool();
    }END_TEST

START_TEST (test_changing_nr_of_threads)
    {
        int n = 2;
        setup_pool( n );

        size_t count = 1000;
        start_threads( &simple_test_thread, &count );

        ck_assert_int_eq( n, get_current_thread_count() );

        struct int_result * result_list[n];

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < n; ++i ) {
            ck_assert_int_eq( count, result_list[i]->val );
        }

        n = 4;
        setup_pool( n );

        start_threads( &simple_test_thread, &count );

        ck_assert_int_eq( n, get_current_thread_count() );

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < n; ++i ) {
            ck_assert_int_eq( count, result_list[i]->val );
        }

        teardown_pool();
    }END_TEST

START_TEST (test_reuse_threadpool)
    {
        int n = 2;
        setup_pool( n );

        size_t count = 1000;
        start_threads( &simple_test_thread, &count );

        ck_assert_int_eq( n, get_current_thread_count() );

        struct int_result * result_list[n];

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < n; ++i ) {
            ck_assert_int_eq( count, result_list[i]->val );
        }

        exit_thread_pool();

        n = 4;
        setup_pool( n );

        start_threads( &simple_test_thread, &count );

        ck_assert_int_eq( n, get_current_thread_count() );

        wait_for_threads( (void **) &result_list );

        for( int i = 0; i < n; ++i ) {
            ck_assert_int_eq( count, result_list[i]->val );
        }

        teardown_pool();
    }END_TEST

void addThreadPoolTC( Suite *s ) {
    TCase *tc_core = tcase_create( "threadpool" );
    tcase_add_test( tc_core, test_one_thread );
    tcase_add_test( tc_core, test_nr_of_cores_threads );
    tcase_add_test( tc_core, test_20_threads );
    tcase_add_test( tc_core, test_1000_threads );
    tcase_add_test( tc_core, test_changing_nr_of_threads );
    tcase_add_test( tc_core, test_reuse_threadpool );

    suite_add_tcase( s, tc_core );
}
