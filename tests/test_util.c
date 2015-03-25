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

#include "tests.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/db_adapter.h"
#include "../src/util/util.h"
#include "../src/util/util_sequence.h"
#include "../src/libssa_extern_db.h"

/**
 * A small helper, to print sequences in decimal representation.
 */
void print_sequence( char* desc, int8_t* seq, size_t len ) {
    outf( "seq (%s): '", desc );
    for( size_t i = 0; i < len; i++ ) {
        outf( "%d", seq[i] );
    }
    outf( "'\n" );
}

void print_sequence2( char* desc, sequence_t seq ) {
    print_sequence( desc, (int8_t *) seq.seq, seq.len );
}

/**
 * Prints the addresses of a list of variables.
 */
void dbg_print( int n, void* param ) {
    int i;
    for( i = 0; i < n; i++ ) {
        outf( "Param %d: %p\n", i, (param + i) );
    }
}

char* concat( char *s1, char *s2 ) {
    char *result = xmalloc( strlen( s1 ) + strlen( s2 ) + 1 ); //+1 for the zero-terminator
    strcpy( result, s1 );
    strcat( result, s2 );
    return result;
}

void ck_converted_prot_eq( char* ref, sequence_t seq ) {
    sequence_t conv_dna;
    conv_dna.len = seq.len;
    conv_dna.seq = xmalloc( seq.len + 1 );
    for( size_t i = 0; i < seq.len; i++ ) {
        conv_dna.seq[i] = sym_ncbi_aa[(int) seq.seq[i]];
    }
    conv_dna.seq[seq.len] = 0;

    ck_assert_str_eq( ref, conv_dna.seq );
}

START_TEST (test_xmalloc)
    {
        uint8_t * arr = xmalloc( 5 );
        for( int i = 0; i < 5; ++i ) {
            arr[i] = 1;
        }

        p_seqinfo * arr2 = xmalloc( 5 * sizeof(p_seqinfo) );
        for( int i = 0; i < 5; ++i ) {
            arr2[i] = xmalloc( sizeof(seqinfo_t) );
            arr2[i]->ID = i;
        }

        for( int i = 0; i < 5; ++i ) {
            free( arr2[i] );
        }
        free( arr2 );
        free( arr );
    }END_TEST

START_TEST (test_xrealloc)
    {
        p_seqinfo * arr = xmalloc( 5 * sizeof(p_seqinfo) );
        for( int i = 0; i < 5; ++i ) {
            arr[i] = xmalloc( sizeof(seqinfo_t) );
            arr[i]->ID = i;
        }

        arr = xrealloc( arr, 10 * sizeof(p_seqinfo) );
        for( int i = 0; i < 10; ++i ) {
            if( i >= 5 ) {
                arr[i] = xmalloc( sizeof(seqinfo_t) );
                arr[i]->ID = i;
            }
        }

        for( int i = 0; i < 10; ++i ) {
            free( arr[i] );
        }
        free( arr );
    }END_TEST

START_TEST (test_add_to_minheap)
    {
        p_minheap heap = minheap_init( 1 );
        int query_id = 0;

        p_sdb_sequence db_seq = xmalloc( sizeof(sdb_sequence_t) );
        db_seq->ID = 42;
        db_seq->strand = 1;
        db_seq->frame = 2;

        long score = 42000;

        add_to_minheap( heap, query_id, db_seq, score );

        ck_assert_int_eq( 1, heap->count );
        ck_assert_int_eq( db_seq->ID, heap->array[0].db_id );
        ck_assert_int_eq( db_seq->strand, heap->array[0].db_strand );
        ck_assert_int_eq( db_seq->frame, heap->array[0].db_frame );
        ck_assert_int_eq( query_id, heap->array[0].query_id );

        minheap_exit( heap );
    }END_TEST

void addUtilTC( Suite *s ) {
    TCase *tc_core = tcase_create( "util" );
    tcase_add_test( tc_core, test_xmalloc );
    tcase_add_test( tc_core, test_xrealloc );
    tcase_add_test( tc_core, test_add_to_minheap );

    suite_add_tcase( s, tc_core );
}
