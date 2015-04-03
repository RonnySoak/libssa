/*
 * benchmark_util.c
 *
 *  Created on: Apr 3, 2015
 *      Author: Jakob Frielingsdorf
 */
#include "benchmark_util.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>

double run_alignment_times( int type, p_query query, size_t hit_count, int bit_width, int internal_iterations ) {
//    p_alignment_list (*align_func)( p_query, size_t, int, int );
//    if( type == SW ) {
//        align_func = &sw_align;
//    }
//    else {
//        align_func = &nw_align;
//    }

    struct timeval start;
    struct timeval finish;

    gettimeofday( &start, NULL );

    for( int i = 0; i < internal_iterations; ++i ) {
//        p_alignment_list alist = align_func( query, hit_count, bit_width, COMPUTE_SCORE );
//        free_alignment( alist ); TODO
    }

    gettimeofday( &finish, NULL );

    double elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_usec - start.tv_usec) / 1000000.0;

    return elapsed;
}

double run_alignment( int type, p_query query, size_t hit_count, int bit_width ) {
    return run_alignment_times( type, query, hit_count, bit_width, 1 );
}

char* concat( char * s1, char * s2 ) {
    char *result = malloc( strlen( s1 ) + strlen( s2 ) + 1 ); //+1 for the zero-terminator
    strcpy( result, s1 );
    strcat( result, s2 );
    return result;
}

p_query read_query( char * query_file ) {
    char * filename = concat( "data/", query_file );
    p_query query = init_sequence_fasta( READ_FROM_FILE, filename );
    free( filename );
    return query;
}

void log_to_file( FILE * f, const char * format, ... ) {
    va_list argptr;
    va_start( argptr, format );
    vfprintf( f, format, argptr );
    va_end( argptr );

    va_start( argptr, format );
    vfprintf( stdout, format, argptr );
    va_end( argptr );
}

FILE * open_log_file( char * desc ) {
    time_t t = time( NULL );
    struct tm tm = *localtime( &t );

    char filename[100];

    sprintf( filename, "results/%d_%d_%d-%d_%d_%s", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min,
            desc );

    FILE *f = fopen( filename, "w" );
    if( f == NULL ) {
        printf( "Error opening file!\n" );
        exit( 1 );
    }
    return f;
}

void run_and_log_n_times( FILE * f, int type, p_query query, int hit_count, int bit_width, int iterations ) {

    for( int i = 0; i < iterations; i++ ) {
        double time = run_alignment( type, query, hit_count, bit_width );

        log_to_file( f, ",%lf", time );
    }
    log_to_file( f, "\n" );
}
