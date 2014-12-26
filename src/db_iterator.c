/*
 * db_iterator.c
 *
 *  Created on: Sep 8, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "db_iterator.h"

#include <stdlib.h>
#include <pthread.h>

#include "util_sequence.h"
#include "libssa_extern_db.h"
#include "util.h"
#include "query.h"
#include "util/thread_pool.h"

static unsigned long chunk_db_seq_count;
static unsigned long next_chunk_start = 0;

static int buffer_max = 0;
static pthread_mutex_t chunk_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Initialises the buffer. Translates the DB sequence and computes the reverse
 * complement of the forward strand, if necessary.
 */
static void set_translated_sequences(p_seqinfo seqinfo, p_sdb_sequence * buffer) {
	sequence db_seq;
    db_seq.seq = seqinfo->seq;
    db_seq.len = seqinfo->seqlen;

    if (symtype == NUCLEOTIDE) {
        // first element

        buffer[0]->ID = seqinfo->ID;
        buffer[0]->seq = us_map_sequence(db_seq, map_ncbi_nt16);
        buffer[0]->strand = 0;
        buffer[0]->frame = 0;

        if (query_strands & 2) {
            // reverse complement
            buffer[1]->ID = seqinfo->ID;
            // no need for a second mapping
            buffer[1]->seq = us_revcompl(buffer[0]->seq);
            buffer[1]->strand = 1;
            buffer[1]->frame = 0;
        }
    }
    else if ((symtype == TRANS_DB) || (symtype == TRANS_BOTH)) {
        // map first and then translate the sequences
        sequence conv_seq = us_map_sequence(db_seq, map_ncbi_nt16);

        if (query_strands == BOTH_STRANDS) {
            for (int s = 0; s < 2; s++) { // strands
                for (int f = 0; f < 3; f++) { // frames
                    buffer[3 * s + f]->ID = seqinfo->ID;

                    buffer[3 * s + f]->strand = s;
                    buffer[3 * s + f]->frame = f;

                    us_translate_sequence(1, conv_seq, s, f, &buffer[3 * s + f]->seq);
                }
            }
        }
        else {
            int s = query_strands - 1;

            for (int f = 0; f < 3; f++) { // frames
                buffer[f]->ID = seqinfo->ID;

                buffer[f]->strand = query_strands;
                buffer[f]->frame = f;

                us_translate_sequence(1, conv_seq, s, f, &buffer[f]->seq);
            }
        }
    }
    else {
        buffer[0]->ID = seqinfo->ID;
        buffer[0]->seq = us_map_sequence(db_seq, map_ncbi_aa);
        buffer[0]->strand = 0;
        buffer[0]->frame = 0;
    }
}

void it_free() {
    buffer_max = 0;
}

void it_init(unsigned long size) {
    // TODO call external DB to set chunk size
    chunk_db_seq_count = size;

    // set buffer size according symtype: 1, 2 oder 6
    if (symtype == NUCLEOTIDE) {
        if (query_strands & 2)
            buffer_max = 2;
        else
            buffer_max = 1;
    }
    else if ((symtype == TRANS_DB) || (symtype == TRANS_BOTH)) {
        if (query_strands ^ BOTH_STRANDS)
            buffer_max = 3;
        else
            buffer_max = 6;
    }
    else {
        buffer_max = 1;
    }
}

p_seqinfo it_get_sequence(unsigned long id) {
    return ssa_db_get_sequence(id);
}

sequence it_translate_sequence(p_seqinfo info, int f, int s) {
    sequence result;

    sequence db_seq;
    db_seq.seq = info->seq;
    db_seq.len = info->seqlen;

    sequence conv_seq = us_map_sequence(db_seq, map_ncbi_nt16);

    if (symtype == NUCLEOTIDE) {
        if (s == 2) {
            result = us_revcompl(conv_seq);
        }
        else {
            result = conv_seq;
        }
    }
    else if ((symtype == TRANS_DB) || (symtype == TRANS_BOTH)) {
        us_translate_sequence(1, conv_seq, s, f, &result);
    }
    else {
        result = conv_seq;
    }

    return result;
}

static void it_free_sequence(p_sdb_sequence seq) {
    if (!seq) {
        return;
    }

    if (&seq->seq) {
        free(seq->seq.seq);
        seq->seq.seq = 0;
        seq->seq.len = 0;
    }

    seq->frame = 0;
    seq->strand = 0;
    seq->ID = 0;

    free(seq);
    seq = 0;
}

void it_free_chunk(p_db_chunk chunk) {
    if (chunk) {
        for (int i = 0; i < chunk->size; i++) {
            it_free_sequence(chunk->seq[i]);
            chunk->seq[i] = 0;
        }

        if (chunk->seq) {
            free(chunk->seq);
            chunk->seq = 0;
        }
        free(chunk);
        chunk = 0;
    }
}

p_db_chunk it_new_chunk() {
	p_db_chunk chunk = xmalloc(sizeof(struct db_chunk));
	chunk->fill_pointer = 0;
	chunk->size = chunk_db_seq_count * buffer_max;
	chunk->seq = xmalloc(chunk->size * sizeof(p_sdb_sequence));

    for (int i = 0; i < chunk->size; ++i) {
        chunk->seq[i] = xmalloc(sizeof(sdb_sequence));

        chunk->seq[i]->seq = (sequence) {0, 0};
        chunk->seq[i]->frame = 0;
        chunk->seq[i]->strand = 0;
        chunk->seq[i]->ID = 0;
    }

	return chunk;
}

void it_reset_chunk_counter() {
	next_chunk_start = 0;
}

void it_next_chunk(p_db_chunk chunk) {
	if (!chunk) {
		ffatal("Chunk not initialized");
	}
	chunk->fill_pointer = 0;

    int next_chunk;

    pthread_mutex_lock(&chunk_mutex);
    next_chunk = next_chunk_start;
    next_chunk_start += chunk_db_seq_count;
    pthread_mutex_unlock(&chunk_mutex);

    for (int i = next_chunk; i < (next_chunk + chunk_db_seq_count); i++) {
    	p_seqinfo db_seq = it_get_sequence(i);

    	if (!db_seq) {
    		break;
    	}

        set_translated_sequences(db_seq, chunk->seq + chunk->fill_pointer);

        chunk->fill_pointer += buffer_max;
    }
}
