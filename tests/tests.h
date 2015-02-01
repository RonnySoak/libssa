/*
 * check_liborsa.h
 *
 *  Created on: 20 Jul 2014
 *      Author: kaos
 */

#ifndef CHECK_LIBORSA_H_
#define CHECK_LIBORSA_H_

#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/libssa_datatypes.h"

void addUtilTC( Suite *s );
void addMinHeapTC( Suite *s );
void addLinkedListTC( Suite *s );
void addThreadPoolTC( Suite *s );
void addQueryTC( Suite *s );
void addMatricesTC( Suite *s );
void addUtilSequenceTC( Suite *s );
void addDBIteratorTC( Suite *s );
void addCigarTC( Suite *s );
void addAlignTC( Suite *s );
void addSmithWaterman63TC( Suite *s );
void addSmithWaterman16TC( Suite *s );
void addNeedlemanWunsch16TC( Suite *s );
void addSmithWaterman8TC( Suite *s );
void addNeedlemanWunsch8TC( Suite *s );
void addSearcher63TC( Suite *s );
void addSearcherTC( Suite *s );
void addManagerTC( Suite *s );
void addAlignerTC( Suite *s );
void addLibssaTC( Suite *s );

// some utility functions
void dbg_print( int n, void* param );
void print_sequence( char* desc, int8_t* seq, long len );
void print_sequence2( char* desc, sequence seq );
char* concat( char *s1, char *s2 );

#endif /* CHECK_LIBORSA_H_ */

