/*
 * db_iterator.c
 *
 *  Created on: Sep 8, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "libssa_extern_db.h"
#include "util.h"

extern void us_translate_sequence(int db_sequence, sequence dna,
        int strand, int frame, sequence * prot_seq);
extern sequence us_revcompl(sequence seq);
extern sequence us_map_sequence(sequence seq, const char* map);

extern const char map_ncbi_nt16[256];
extern const char map_ncbi_aa[256];

static unsigned long seq_index;
static unsigned long chunk_size;

static int buffer_max = 0;
static p_sdb_sequence* buffer;
static int buffer_p = 0;

/**
 * Initialises the buffer. Translates the DB sequence and computes the reverse
 * complement of the forward strand, if necessary.
 */
static void fill_buffer(p_seqinfo seqinfo) {
    sequence db_seq;
    db_seq.seq = seqinfo->seq;
    db_seq.len = seqinfo->seqlen;

    if (symtype == NUCLEOTIDE) {
        // first element

        buffer[0] = (p_sdb_sequence )xmalloc(sizeof(sdb_sequence));
        buffer[0]->ID = seqinfo->ID;
        buffer[0]->seq = us_map_sequence(db_seq, map_ncbi_nt16);
        buffer[0]->strand = 0;
        buffer[0]->frame = 0;

        if (query_strands & 2) {
            // reverse complement
            buffer[1] = (p_sdb_sequence )xmalloc(sizeof(sdb_sequence));
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

        if (query_strands ^ BOTH_STRANDS) {
            // for forward or complementary strand only
            int s = query_strands - 1;

            for (int f = 0; f < 3; f++) { // frames
                buffer[f] = (p_sdb_sequence )xmalloc(sizeof(sdb_sequence));
                buffer[f]->ID = seqinfo->ID;

                buffer[f]->strand = query_strands;
                buffer[f]->frame = f;

                us_translate_sequence(1, conv_seq, s, f, &buffer[f]->seq);
            }
        }
        else {
            // for both strands translation
            for (int s = 0; s < 2; s++) { // strands
                for (int f = 0; f < 3; f++) { // frames
                    buffer[3 * s + f] = (p_sdb_sequence )xmalloc(sizeof(sdb_sequence));
                    buffer[3 * s + f]->ID = seqinfo->ID;

                    buffer[3 * s + f]->strand = s;
                    buffer[3 * s + f]->frame = f;

                    us_translate_sequence(1, conv_seq, s, f, &buffer[3 * s + f]->seq);
                }
            }
        }
    }
    else {
        buffer[0] = (p_sdb_sequence )xmalloc(sizeof(sdb_sequence));
        buffer[0]->ID = seqinfo->ID;
        buffer[0]->seq = us_map_sequence(db_seq, map_ncbi_aa);
        buffer[0]->strand = 0;
        buffer[0]->frame = 0;
    }
}

void it_free() {
    buffer_max = 0;
    buffer_p = 0;

    if (buffer) {
        free(buffer);
        buffer = 0;
    }
}

void it_init(unsigned long size) {
    // TODO call external DB to set chunk size
    chunk_size = size;

    // reset the DB sequence index
    seq_index = 0;

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

    // at first we start with a "full" buffer, to force a sequence loading!
    buffer_p = buffer_max;

    buffer = xmalloc(buffer_max * sizeof(p_sdb_sequence));

    for (int i = 0; i < buffer_max; i++) {
        buffer[i] = 0;
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

/**
 * Returns the next sequence from the database. Translates the DB sequences, if
 * necessary, and returns the translated sequences one by one.
 *
 * @return the next sequence in the DB, or NULL if none is left
 */
p_sdb_sequence it_next_sequence() {
    if (buffer_p < buffer_max) {
        return buffer[buffer_p++];
    }

    // get sequence from DB
    p_seqinfo seqinfo = ssa_db_next_sequence();
    if (!seqinfo) {
        // last sequence read
        return NULL;
    }

    // translate sequence according sym-type
    fill_buffer(seqinfo);

    // return current version
    buffer_p = 0;
    return buffer[buffer_p++];
}

void it_free_sequence(p_sdb_sequence seq) {
    if (!seq) {
        return;
    }

    if (seq->seq.seq) {
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
        for (int i = 0; i < chunk_size; i++) {
            if (chunk->seq[i]) {
                chunk->seq[i] = 0;
            }
        }
        if (chunk->seq) {
            free(chunk->seq);
            chunk->seq = 0;
        }
        free(chunk);
        chunk = 0;
    }
}

p_db_chunk it_next_chunk() {
    p_db_chunk chunk = (p_db_chunk) xmalloc(sizeof(struct db_chunk));

    chunk->seq = (p_sdb_sequence *) xmalloc(chunk_size * sizeof(p_sdb_sequence));
    chunk->size = 0;

    for (int i = 0; i < chunk_size; i++) {
        chunk->seq[i] = 0;
    }

    // TODO make it better and move it to the DB
    for (int i = 0; i < chunk_size; i++) {
        chunk->seq[chunk->size] = it_next_sequence();
        if (!chunk->seq[chunk->size]) {
            break;
        }

        chunk->size++;
    }

    if (!chunk->size) {
        it_free_chunk(chunk);
        return NULL;
    }
    return chunk;
}

