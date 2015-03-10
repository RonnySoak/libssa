/*
 * thread_pool.c
 *
 *  Created on: Oct 20, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "thread_pool.h"

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include <stdio.h> // TODO remove and change output to outf function in util.c

#include "../libssa.h"
#include "util.h"

size_t max_thread_count = -1;

static pthread_t * thread_list = 0;
static size_t current_thread_count = 0;

size_t get_current_thread_count() {
    return current_thread_count;
}

static size_t get_new_thread_count() {
    if( !current_thread_count || (current_thread_count != max_thread_count) ) {
        if( max_thread_count == -1 ) {
            max_thread_count = get_nprocs(); //sysconf(_SC_NPROCESSORS_ONLN);
        }

        current_thread_count = max_thread_count;
    }
    return current_thread_count;
}

void init_thread_pool() {
    if( thread_list ) {
        if( get_current_thread_count() == max_thread_count ) {
            return;
        }
        exit_thread_pool();
    }

    size_t thread_count = get_new_thread_count();

    outf( "Using %ld threads\n", thread_count );

    thread_list = xmalloc( thread_count * sizeof(pthread_t) );
}

void exit_thread_pool() {
    if( thread_list ) {
//        for (int i = 0; i < _max_thread_count; i++) {
//            pthread_cancel(thread_list[i]); TODO implement termination of unfinished threads
//        }

        free( thread_list );
        thread_list = 0;
    }
}

void start_threads( void *(*start_routine)( void * ), void * arg ) {
    for( size_t i = 0; i < get_current_thread_count(); i++ ) {
        pthread_create( &thread_list[i], NULL, start_routine, arg );
    }
}

void wait_for_threads( void ** thread_results ) {
    for( size_t i = 0; i < max_thread_count; i++ ) {
        pthread_join( thread_list[i], &thread_results[i] );
    }
}
