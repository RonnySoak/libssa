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

#ifndef SEARCH_64_H_
#define SEARCH_64_H_

#include <stdint.h>

#include "../../libssa_datatypes.h"

void search_64_init_algo( int search_type );

int64_t* search_64_alloc_hearray( p_search_data sdp );

int64_t full_sw(sequence_t * dseq, sequence_t * qseq, int64_t * hearray );

int64_t full_nw(sequence_t * dseq, sequence_t * qseq, int64_t * hearray );

int64_t full_nw_sellers(sequence_t * dseq, sequence_t * qseq, int64_t * hearray );

void search_64_chunk( p_minheap heap, p_db_chunk chunk, p_search_data sdp, int64_t* hearray );

void search_64( p_db_chunk chunk, p_search_data sdp, p_search_result res );

#endif /* SEARCH_64_H_ */
