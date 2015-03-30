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

#include "util.h"

#include <stdarg.h>
#include <mm_malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "../db_adapter.h"

/*
 * For AVX2 instructions memory needs to be 32 byte aligned.
 */
#define ALIGNMENT 32

int output_mode = OUTPUT_STDOUT;

void ffatal( const char * format, ... ) {
    if( format ) {
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stderr, format, argptr);
        vfprintf( stderr, "\n", 0 );
        va_end(argptr);
    }
    exit( 1 );
}

void outf( const char * format, ... ) {
    if( output_mode == OUTPUT_STDOUT ) {
        va_list argptr;
        va_start(argptr, format);
        vfprintf(stdout, format, argptr);
        va_end(argptr);
    }
}

void * xmalloc( size_t size ) {
    void * t;
    if( posix_memalign( &t, ALIGNMENT, size ) != 0 ) {
        ffatal( "Unable to allocate enough memory: %ld.", size );
    }

    return t;
}

void * xrealloc( void *ptr, size_t size ) {
    void * t = realloc( ptr, size );
    if( !t ) {
        ffatal( "Unable to re-allocate enough memory: %ld.", size );
    }
    return t;
}

void add_to_minheap( p_minheap heap, uint8_t query_id, p_sdb_sequence db_seq, long score ) {
    elem_t e;
    e.query_id = query_id;
    e.db_id = db_seq->ID;
    e.db_frame = db_seq->frame;
    e.db_strand = db_seq->strand;
    e.score = score;

#ifdef DBG_COLLECT_ALIGNED_DB_SEQUENCES
    dbg_add_aligned_sequence( db_seq->ID, query_id, score );
#endif

    /*
     * Alignments, with a score equal to the current lowest score in the heap are ignored
     */
    minheap_add( heap, &e );
}
