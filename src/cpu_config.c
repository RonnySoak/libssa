/*
 * cpu_config.c
 *
 *  Created on: Feb 4, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "cpu_config.h"

#include "util/util.h"

/*
 * We can have features, that are
 *  (a) set at compile time
 *  (b) implemented by the present CPU
 *  (c) set at runtime
 *
 * (a) have to be checked against what the current CPU has implemented
 * (a) can be changed at runtime, but cannot enable features, that have been disabled at compile time
 *
 * (b) define an initial set of features
 * (b) used features can be changed at runtime
 *
 * (c) have to be checked against what the current CPU has implemented
 * (c) cannot enable features, that have been disabled at compile time
 *
 * The following implementation chooses the SIMD implementation at runtime. At default
 * all is enabled and the user can set a max compute capability, disabling all higher ones.
 */

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
        ffatal( "libssa was set to use SSE2, but CPU does not support SSE2." );
    }
    if( sse41_enabled && !__builtin_cpu_supports( "sse4.1" ) ) {
        ffatal( "libssa was set to use SSE4.1, but CPU does not support SSE4.1." );
    }
    if( avx2_enabled && !__builtin_cpu_supports( "avx2" ) ) {
        ffatal( "libssa was set to use AVX2, but CPU does not support AVX2." );
    }
}
