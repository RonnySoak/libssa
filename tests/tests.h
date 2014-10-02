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

void addSearch63TC(Suite *s);
void addSearcherTC(Suite *s);
void addQueryTC(Suite *s);
void addMatricesTC(Suite *s);
void addUtilSequenceTC(Suite *s);
void addDBIteratorTC(Suite *s);
void addMinHeapTC(Suite *s);

#endif /* CHECK_LIBORSA_H_ */

