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

#include "db_adapter.h"

#include <stdlib.h>
#include <pthread.h>

#include "util/util_sequence.h"
#include "libssa_extern_db.h"
#include "util/util.h"
#include "query.h"
#include "util/thread_pool.h"

static size_t chunk_db_seq_count = 0;
static size_t next_chunk_start = 0;

static int buffer_max = 0;
static pthread_mutex_t chunk_mutex = PTHREAD_MUTEX_INITIALIZER;

static void realloc_sequence( sequence_t * seq, size_t len ) {
    seq->seq = xrealloc( seq->seq, len + 1 );
    seq->len = len;
}

/**
 * Initialises the buffer. Translates the DB sequence and computes the reverse
 * complement of the forward strand, if necessary.
 */
static void set_translated_sequences( p_seqinfo seqinfo, p_sdb_sequence * buffer ) {
    sequence_t db_seq;
    db_seq.seq = seqinfo->seq;
    db_seq.len = seqinfo->seqlen;

    if( symtype == NUCLEOTIDE ) {
        // first element

        buffer[0]->ID = seqinfo->ID;
        realloc_sequence( &buffer[0]->seq, db_seq.len );
        us_map_sequence( db_seq, buffer[0]->seq, map_ncbi_nt16 );
        buffer[0]->strand = 0;
        buffer[0]->frame = 0;

        if( query_strands & 2 ) {
            // reverse complement
            buffer[1]->ID = seqinfo->ID;
            // no need for a second mapping
            realloc_sequence( &buffer[1]->seq, db_seq.len );

            us_revcompl( buffer[0]->seq, buffer[1]->seq );
            buffer[1]->strand = 1;
            buffer[1]->frame = 0;
        }
    }
    else if( (symtype == TRANS_DB) || (symtype == TRANS_BOTH) ) {
        // map first and then translate the sequences
        sequence_t conv_seq = { xmalloc( db_seq.len + 1 ), db_seq.len };

        us_map_sequence( db_seq, conv_seq, map_ncbi_nt16 );

        if( query_strands == BOTH_STRANDS ) {
            for( int s = 0; s < 2; s++ ) { // strands
                for( int f = 0; f < 3; f++ ) { // frames
                    buffer[3 * s + f]->ID = seqinfo->ID;

                    buffer[3 * s + f]->strand = s;
                    buffer[3 * s + f]->frame = f;

                    us_translate_sequence( 1, conv_seq, s, f, &buffer[3 * s + f]->seq );
                }
            }
        }
        else {
            int s = query_strands - 1;

            for( int f = 0; f < 3; f++ ) { // frames
                buffer[f]->ID = seqinfo->ID;

                buffer[f]->strand = query_strands;
                buffer[f]->frame = f;

                us_translate_sequence( 1, conv_seq, s, f, &buffer[f]->seq );
            }
        }
    }
    else {
        buffer[0]->ID = seqinfo->ID;
        realloc_sequence( &   buffer[0]->seq, db_seq.len );
        us_map_sequence( db_seq, buffer[0]->seq, map_ncbi_aa );
        buffer[0]->strand = 0;
        buffer[0]->frame = 0;
    }
}

void adp_exit() {
    buffer_max = 0;
    chunk_db_seq_count = 0;
    next_chunk_start = 0;
}

void adp_init( size_t size ) {
    chunk_db_seq_count = size;

    // set buffer size according symtype: 1, 2, 3 oder 6
    if( symtype == NUCLEOTIDE ) {
        if( query_strands & 2 )
            buffer_max = 2;
        else
            buffer_max = 1;
    }
    else if( (symtype == TRANS_DB) || (symtype == TRANS_BOTH) ) {
        if( query_strands ^ BOTH_STRANDS )
            buffer_max = 3;
        else
            buffer_max = 6;
    }
    else {
        buffer_max = 1;
    }
}

void adp_free_sequence( p_sdb_sequence seq ) {
    if( !seq ) {
        return;
    }

    if( &seq->seq ) {
        free( seq->seq.seq );
        seq->seq.seq = 0;
        seq->seq.len = 0;
    }

    seq->frame = 0;
    seq->strand = 0;
    seq->ID = 0;

    free( seq );
    seq = 0;
}

void adp_free_chunk_no_sequences( p_db_chunk chunk ) {
    if( chunk ) {
        if( chunk->seq ) {
            free( chunk->seq );
            chunk->seq = 0;
        }
        free( chunk );
        chunk = 0;
    }
}

void adp_free_chunk( p_db_chunk chunk ) {
    if( chunk ) {
        for( size_t i = 0; i < chunk->size; i++ ) {
            adp_free_sequence( chunk->seq[i] );
            chunk->seq[i] = 0;
        }

        if( chunk->seq ) {
            free( chunk->seq );
            chunk->seq = 0;
        }
        free( chunk );
        chunk = 0;
    }
}

p_db_chunk adp_alloc_chunk( size_t size ) {
    p_db_chunk chunk = xmalloc( sizeof(db_chunk_t) );
    chunk->fill_pointer = 0;
    chunk->size = size;
    chunk->seq = xmalloc( chunk->size * sizeof(p_sdb_sequence) );

    return chunk;
}

p_db_chunk adp_init_new_chunk() {
    p_db_chunk chunk = adp_alloc_chunk( chunk_db_seq_count * buffer_max );

    for( size_t i = 0; i < chunk->size; ++i ) {
        chunk->seq[i] = xmalloc( sizeof(sdb_sequence_t) );

        chunk->seq[i]->seq = (sequence_t ) { xmalloc( 1 ), 0 };
        chunk->seq[i]->seq.seq[0] = 0;
        chunk->seq[i]->frame = 0;
        chunk->seq[i]->strand = 0;
        chunk->seq[i]->ID = 0;
    }

    return chunk;
}

void adp_reset_chunk_counter() {
    next_chunk_start = 0;
}

void adp_next_chunk( p_db_chunk chunk ) {
    if( !chunk ) {
        ffatal( "Chunk not initialized" );
    }
    chunk->fill_pointer = 0;

    size_t next_chunk;

    pthread_mutex_lock( &chunk_mutex );
    next_chunk = next_chunk_start;
    next_chunk_start += chunk_db_seq_count;
    pthread_mutex_unlock( &chunk_mutex );

    for( size_t i = next_chunk; i < (next_chunk + chunk_db_seq_count); i++ ) {
        p_seqinfo db_seq = ssa_db_get_sequence( i );

        if( !db_seq ) {
            break;
        }

        if( db_seq->seqlen == 0 ) {
            continue;
        }

        set_translated_sequences( db_seq, chunk->seq + chunk->fill_pointer );

        chunk->fill_pointer += buffer_max;
    }
}
