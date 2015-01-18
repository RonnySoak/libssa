/*
 * aligner.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef ALIGNER_H_
#define ALIGNER_H_

#include "../libssa_datatypes.h"

void a_init_data( int search_type );

void a_free_data();

void a_free(p_alignment_list alist);

void a_set_alignment_pairs( int pair_count, elem_t * result_sequence_pairs );

void * a_align(void * adp);

#endif /* ALIGNER_H_ */
