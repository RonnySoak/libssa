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

#include "../libssa.h"
#include "../util.h"

static pthread_t * thread_list = 0;
static long current_thread_count = 0;

int get_current_thread_count() {
    return current_thread_count;
}

void init_thread_pool() {
    if (thread_list && (_max_thread_count == get_nprocs())) {
        return;
    }

    exit_thread_pool();

    if (_max_thread_count == -1) {
        /*
         * TODO move this into a general config initializer
         */
        _max_thread_count = get_nprocs();//sysconf(_SC_NPROCESSORS_ONLN);
    }

    printf("Using %ld threads\n", _max_thread_count);

    thread_list = xmalloc(_max_thread_count * sizeof(pthread_t));
}

void exit_thread_pool() {
    if (thread_list) {
//        for (int i = 0; i < _max_thread_count; i++) {
//            pthread_cancel(thread_list[i]); TODO implement termination of unfinished threads
//        }

        free(thread_list);
    }
}

void start_threads_unified_arguments(void *(*start_routine) (void *),
        void * thread_argument) {
    current_thread_count = _max_thread_count;

    for (int i = 0; i < current_thread_count; i++) {
        pthread_create(&thread_list[i], NULL, start_routine, thread_argument);
    }
}

void wait_for_threads(void ** thread_results) {
    for (int i = 0; i < _max_thread_count; i++) {
        pthread_join(thread_list[i], &thread_results[i]);
    }
}
