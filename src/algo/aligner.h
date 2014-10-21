/*
 * aligner.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef ALIGNER_H_
#define ALIGNER_H_

#include "../libssa_datatypes.h"

void a_free(p_alignment_list alist);

void set_alignment_data(p_alignment_data data);

void * a_align(void * adp);

#endif /* ALIGNER_H_ */
