/*
 Copyright (C) 2014-2015 Jakob Frielingsdorf

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Contact: Jakob Frielingsdorf <jfrielingsdorf@gmail.com>
 */

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <stddef.h>

extern size_t max_thread_count;

void init_thread_pool();

void exit_thread_pool();

size_t get_current_thread_count();

void start_threads( void *(*start_routine)( void * ), void * arg );

void wait_for_threads( void ** thread_results );

#endif /* THREAD_POOL_H_ */
