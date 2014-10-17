/*
 * aligner.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef ALIGNER_H_
#define ALIGNER_H_

#include "../libssa.h"

void a_free(p_alignment_list alist);

p_alignment_list a_align(p_minheap heap, seq_buffer* queries, int q_count);

#endif /* ALIGNER_H_ */
