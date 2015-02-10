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
#include "../db_iterator.h"

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
//    if( !out_file ) { // TODO test it, is broken
//        init_out( NULL );
//    }

    va_list arg;
    printf( format, arg );
}

void * xmalloc( size_t size ) {
    const size_t alignment = 32; // TODO AVX2 code needs a 32 bit alignment ...
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

void add_to_minheap( p_minheap heap, int query_id, p_sdb_sequence db_seq, long score ) {
    elem_t * e = xmalloc( sizeof(elem_t) );
    e->query_id = query_id;

    e->db_id = db_seq->ID;
    e->dframe = db_seq->frame;
    e->dstrand = db_seq->strand;
    e->score = score;

#ifdef DBG_COLLECT_ALIGNED_DB_SEQUENCES
    dbg_add_aligned_sequence( db_seq->ID, query_id, score );
#endif

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

p_db_chunk convert_to_chunk( p_node linked_list ) {
    p_node node = linked_list;

    unsigned long size = ll_size( node );

    p_db_chunk chunk = it_alloc_chunk( size );

    while( node ) {
        chunk->seq[chunk->fill_pointer++] = ll_pop( &node );
    }

    return chunk;
}

#endif /* DATA_H_ */
