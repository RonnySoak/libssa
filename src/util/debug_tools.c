/*
 * debug_tools.c
 *
 *  Created on: Jan 28, 2015
 *      Author: Jakob Frielingsdorf
 */

#include "debug_tools.h"

#include "stdio.h"

#include "util_sequence.h"

void dbg_mm_print_u( char * desc, __m128i x ) {
    unsigned short * y = (unsigned short*) &x;

    printf( "%s: ", desc );

    for( int i = 0; i < 8; i++ )
        printf( "%s%6d", (i > 0 ? " " : ""), y[7 - i] );
    printf( "\n" );
}

void dbg_mm_print_s( char * desc, __m128i x ) {
    signed short * y = (signed short*) &x;

    printf( "%s: ", desc );

    for( int i = 0; i < 8; i++ ) {
        printf( "%s%2d", (i > 0 ? " " : ""), y[7 - i] );
    }
    printf( "\n" );
}

void dbg_dprofile_dump16( int16_t * dprofile, int cdepth, int channels ) {
    const char * s = sym_ncbi_nt16u;
    printf( "\ndprofile:\n" );
    for( int i = 0; i < 16; i++ ) {
        printf( "%c: ", s[i] );
        for( int k = 0; k < cdepth; k++ ) {
            printf( "[" );
            for( int j = 0; j < channels; j++ )
                printf( " %3d", dprofile[channels * cdepth * i + channels * k + j] );
            printf( "]" );
        }
        printf( "\n" );
    }
}

void dbg_dumpscorematrix( int16_t * m ) {
    for( int i = 0; i < 16; i++ ) {
        printf( "%2d %c", i, sym_ncbi_nt16u[i] );
        for( int j = 0; j < 16; j++ )
            printf( " %2d", m[16 * i + j] );
        printf( "\n" );
    }
}
