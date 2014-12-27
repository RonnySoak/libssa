/*
 * util.c
 *
 *  Created on: 18 Jul 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef DATA_H_
#define DATA_H_

#include <stdarg.h>
#include <mm_malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

FILE* out_file;

void ffatal( const char * format, ... ) {
    if( format ) {
        va_list arg;
        fprintf( stderr, format, arg );
        fprintf( stderr, "\n" );
    }
    exit( 1 );
}

void init_out( const char* filename ) {
    if( NULL == filename ) {
        out_file = stdout;
        outf( "Writing to stdout\n" );
    }
    else {
        FILE * f = fopen( filename, "w" );
        if( !f ) {
            ffatal( "Unable to open output file for writing." );
        }
        out_file = f;
    }
}

void close_out() {
    if( out_file && (out_file != stdout) ) {
        if( fclose( out_file ) ) {
            ffatal( "Could not close output file." );
        }
    }
}

void outf( const char* format, ... ) {
    if( !out_file ) { // TODO test it, is broken
        init_out( NULL );
    }

    va_list arg;
    fprintf( out_file, format, arg );
}

void * xmalloc( size_t size ) {
    const size_t alignment = 16;
    void * t;
    if( posix_memalign( &t, alignment, size ) != 0 ) {
        ffatal( "Unable to allocate enough memory." );
    }

    return t;
}

void * xrealloc( void *ptr, size_t size ) {
    void * t = realloc( ptr, size );
    if( !t ) {
        ffatal( "Unable to allocate enough memory." );
    }
    return t;
}

char *strdup( const char *str ) {
    int n = strlen( str ) + 1;
    char *dup = malloc( n );
    if( dup ) {
        strcpy( dup, str );
    }
    return dup;
}

void add_to_minheap( p_minheap heap, int query_id, p_sdb_sequence db_seq, long score ) {
    elem_t * e = xmalloc( sizeof(elem_t) );
    e->query_id = query_id;

    e->db_id = db_seq->ID;
    e->dframe = db_seq->frame;
    e->dstrand = db_seq->strand;
    e->score = score;

    /* Alignments, with a score equal to the current lowest score in the
     heap are ignored! */
    minheap_add( heap, e );

    /*
     * minheap_add dereferences e and stores a copy of e, if its score
     * is higher than the lowest score in the heap.
     *
     * This means, we can and should free e here!
     */
    free( e );
}

#endif /* DATA_H_ */
