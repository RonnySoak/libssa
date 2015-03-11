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

#ifndef SRC_CPU_CONFIG_H_
#define SRC_CPU_CONFIG_H_

void set_max_compute_capability( int capability );

void reset_compute_capability();

void test_cpu_features();

int is_sse2_enabled();
int is_sse41_enabled();
int is_avx2_enabled();

#endif /* SRC_CPU_CONFIG_H_ */
