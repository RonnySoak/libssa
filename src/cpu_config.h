/*
 * cpu_config.h
 *
 *  Created on: Feb 4, 2015
 *      Author: Jakob Frielingsdorf
 */

#ifndef SRC_CPU_CONFIG_H_
#define SRC_CPU_CONFIG_H_

void set_max_compute_capability( int capability );

void reset_compute_capability();

void test_cpu_features();

int is_sse2_enabled();
int is_sse41_enabled();
int is_avx2_enabled();

#endif /* SRC_CPU_CONFIG_H_ */
