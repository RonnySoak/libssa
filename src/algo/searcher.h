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

#ifndef SEARCHER_H_
#define SEARCHER_H_

#include "../libssa_datatypes.h"

void s_init( int search_type, int bit_width, p_query query );

p_search_data s_create_searchdata( p_query query );

void s_free_search_data( p_search_data sdp );

int s_get_query_count();

seq_buffer_t s_get_query( int idx );

void s_free();

void * s_search( void * hit_count );

#endif /* SEARCHER_H_ */
