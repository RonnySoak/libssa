/*
 * libssa.c
 *
 * LIBSSA - Library for SIMD accelerated optimal Sequence Alignments
 *
 *  Created on: 28 Aug 2014
 *      Author: Jakob Frielingsdorf
 */

#include "libssa.h"

#include "util/util.h"
#include "util/util_sequence.h"
#include "matrices.h"
#include "algo/manager.h"
#include "algo/aligner.h"
#include "query.h"
#include "util/thread_pool.h"
#include "db_iterator.h"
#include "cpu_config.h"

// #############################################################################
// Configuration data
// ##################
size_t _max_thread_count = -1;
int _output_mode = OUTPUT_STDOUT;

// #############################################################################
// Alignment data
// ##################
uint8_t gapO = 0;
uint8_t gapE = 0;

// #############################################################################
// Technical initialisation
// ########################
void set_output_mode( int mode ) {
    _output_mode = mode;
}

void set_threads( size_t nr ) {
    _max_thread_count = nr;
}

// #############################################################################
// Initialisations
// ################
/**
 * Initialises the used scoring matrix (e.g. BLOSUM62, PAM250, ...). The data
 * is stored internally.
 *
 * @param matrix_name  name of the matrix to initialise
 *
 * List of available matrices:
 *  - blosum45
 *  - blosum50
 *  - blosum62
 *  - blosum80
 *  - blosum90
 *  - pam30
 *  - pam70
 *  - pam250
 */
void init_score_matrix( int mode, const char* matrix ) {
    if( READ_FROM_FILE == mode ) {
        mat_init_from_file( matrix );
    }
    else if( READ_FROM_STRING == mode ) {
        mat_init_from_string( matrix );
    }
    else if( MATRIX_BUILDIN == mode ) {
        mat_init_buildin( matrix );
    }
    else {
        ffatal( "Unknown mode for reading score matrices: %d", mode );
    }
}

/**
 * Initialises gap penalties used for the alignments.
 *
 * @param  gapO  penalty for opening a gap
 * @param  gapE  penalty for extending a gap
 */
void init_gap_penalties( const uint8_t gap_open, const uint8_t gap_extend ) {
    gapO = gap_open;
    gapE = gap_extend;
}

/**
 * Initialises the scoring scheme if no scoring matrix is used.
 *
 * Should only be used, if no scoring matrix is used.
 *
 * @param  p    penalty for a mismatch
 * @param  m    reward for a match
 */
void init_constant_scoring( const int8_t p, const int8_t m ) {
    mat_init_constant_scoring( p, m );
}

/**
 * Reads a FASTA file containing multiple sequences to compare the query
 * sequence against.
 * The initialised data is stored internally.
 *
 * @param fasta_db_file  path to a file in FASTA format
 */
void init_db_fasta( const char* fasta_db_file ) {
    ssa_db_free();

    ssa_db_init_fasta( fasta_db_file );

    outf( "DB read %lu sequences\n", ssa_db_get_sequence_count() );
}

/**
 * Default is DEFAULT_SYMTYPE = AMINOACID (1)
 *
 * the symbol translation is done on the fly on both sides
 */
void init_symbol_translation( int type, int strands, int d_gencode, int q_gencode ) {
    if( (q_gencode < 1) || (q_gencode > 23) || !gencode_names[q_gencode - 1] ) {
        ffatal( "Illegal query genetic code specified." );
    }
    if( (d_gencode < 1) || (d_gencode > 23) || !gencode_names[d_gencode - 1] ) {
        ffatal( "Illegal database genetic code specified." );
    }
    if( (symtype < 0) || (symtype > 4) ) {
        ffatal( "Illegal symbol type specified." );
    }
    if( (symtype < 0) || (symtype > 2) ) {
        ffatal( "Illegal strands specified." );
    }

    symtype = type;
    query_strands = strands;

    us_init_translation( q_gencode, d_gencode );
}

/**
 * Reads a FASTA file containing the query sequence.
 *
 * @param fasta_seq_file  path to a file in FASTA format
 * @return pointer to the query profile structure
 */
p_query init_sequence_fasta( int mode, const char* fasta_sequence ) {
    p_query query = NULL;

    if( READ_FROM_FILE == mode ) {
        query = query_read_from_file( fasta_sequence );
    }
    else if( READ_FROM_STRING == mode ) {
        // TODO do we need the header here?
        query = query_read_from_string( "\0", fasta_sequence );
    }
    else {
        ffatal( "Unknown mode for reading query sequences: %d", mode );
    }

    return query;
}

/**
 * Release the memory allocated by the functions init_sequence_fasta.
 *
 * @param  p   pointer to the query profile structure
 *
 * @see init_sequence_fasta
 */
void free_sequence( p_query p ) {
    query_free( p );
}

// #############################################################################
// Alignment
// #########
static void test_configuration() {
    test_cpu_features();

    if( !gapO && !gapE ) {
        ffatal( "Gap opening and gap extension cost set to zero. Possible error." );
    }
    if( !score_matrix_63 ) {
        ffatal( "Scoring not initialized." );
    }

    // TODO extend these checks
}

static void prepare_alignment() {
    test_configuration();

    it_reset_chunk_counter();
}

/**
 * Aligns the query sequence against all sequences in the database using the
 * Smith-Waterman Algorithm.
 *
 * @param  p   pointer to the query profile structure
 * ...
 * @return pointer to the alignment structure
 */
p_alignment_list sw_align( p_query p, size_t hitcount, int bit_width, int align_type /* TODO ...*/) {
    prepare_alignment();

    init_for_sw( p, hitcount, bit_width, align_type );

    return m_run();
}

/**
 * Aligns the query sequence against all sequences in the database using the
 * Needleman-Wunsch Algorithm.
 *
 * @param  p   pointer to the query profile structure
 * ...
 * @return pointer to the alignment structure
 */
p_alignment_list nw_align( p_query p, size_t hitcount, int bit_width, int align_type /* TODO ...*/) {
    prepare_alignment();

    init_for_nw( p, hitcount, bit_width, align_type );

    return m_run();
}

/**
 * Aligns the query sequence against all sequences in the database using the
 * Needleman-Wunsch Algorithm as described by Sellers (TODO ref).
 *
 * TODO This function might call nw_align in the background.
 *
 * @param  p   pointer to the query profile structure
 * ...
 * @return pointer to the alignment structure
 */
p_alignment_list nw_sellers_align( p_query p, size_t hitcount, int bit_width, int align_type /* TODO ...*/) {
    prepare_alignment();

    return NULL; // TODO
}

/**
 * Aligns the query sequence against all sequences in the database using the
 * Needleman-Wunsch Algorithm ignoring gaps in the beginning and/or end.
 *
 * @param  p   pointer to the query profile structure
 * ...
 * @return pointer to the alignment structure
 */
p_alignment_list nw_ignore_gaps_align( p_query p, size_t hitcount, int bit_width, int align_type /* TODO ... ignored gaps...*/) {
    prepare_alignment();

    return NULL; // TODO
}

/**
 * Release the memory allocated by the functions sw_align, nw_align,
 * nw_sellers_align and nw_ignore_gaps_align.
 *
 * @param  a   pointer to the alignment structure
 *
 * @see sw_align
 * @see nw_align
 */
void free_alignment( p_alignment_list alist ) {
    a_free( alist );
}

void ssa_exit() {
    mat_free();
    ssa_db_free();

    exit_thread_pool();
}
