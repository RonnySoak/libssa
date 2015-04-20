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

#include "cpu_config.h"

#include "libssa.h"
#include "util/util.h"

static int sse2_enabled = 1;
static int sse41_enabled = 1;
static int avx2_enabled = 1;

void set_max_compute_capability( int capability ) {
    sse2_enabled = capability >= COMPUTE_ON_SSE2;
    sse41_enabled = capability >= COMPUTE_ON_SSE41;
    avx2_enabled = capability >= COMPUTE_ON_AVX2;
}

void reset_compute_capability() {
    sse2_enabled = 1;
    sse41_enabled = 1;
    avx2_enabled = 1;
}

int is_sse2_enabled() {
    return sse2_enabled;
}

int is_sse41_enabled() {
    return sse41_enabled;
}

int is_avx2_enabled() {
    return avx2_enabled;
}

void test_cpu_features() {
    __builtin_cpu_init();

    if( sse2_enabled && !__builtin_cpu_supports( "sse2" ) ) {
        fatal( "libssa was set to use SSE2, but CPU does not support SSE2." );
    }
    if( sse41_enabled && !__builtin_cpu_supports( "sse4.1" ) ) {
        fatal( "libssa was set to use SSE4.1, but CPU does not support SSE4.1." );
    }
    if( avx2_enabled && !__builtin_cpu_supports( "avx2" ) ) {
        fatal( "libssa was set to use AVX2, but CPU does not support AVX2." );
    }
}
