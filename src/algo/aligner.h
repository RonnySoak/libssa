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

#ifndef ALIGNER_H_
#define ALIGNER_H_

#include "../libssa_datatypes.h"

void a_init_data( int search_type );

void a_free_data();

void a_free( p_alignment_list alist );

void a_set_alignment_pairs( size_t pair_count, elem_t * result_sequence_pairs );

void create_score_alignment_list( p_minheap search_results, p_alignment_list alist );

void * a_align( void * adp );

#endif /* ALIGNER_H_ */
