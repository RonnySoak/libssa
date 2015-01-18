/*
 * thread_pool.h
 *
 *  Created on: Oct 20, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

void init_thread_pool();

void exit_thread_pool();

int get_current_thread_count();

void start_threads(void *(*start_routine) (void *) );

void wait_for_threads(void ** thread_results);

#endif /* THREAD_POOL_H_ */
