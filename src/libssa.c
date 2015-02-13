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
#include "algo/searcher.h"
#include "query.h"
#include "util/thread_pool.h"
#include "db_iterator.h"
#include "cpu_config.h"

// #############################################################################
// Configuration data
// ##################
int _verbose = VERBOSE_OFF;
size_t _max_thread_count = -1;
int _use_simd = SIMD_ON;

// #############################################################################
// Data types
// ##########

// defined in and include from libssa_datatypes.h

uint8_t gapO = 0;
uint8_t gapE = 0;

// #############################################################################
// Technical initialisation
// ########################
void set_verbose( int debug ) {
    _verbose = debug;
}

void set_threads( size_t nr ) {
    _max_thread_count = nr;
}

void set_use_simd( int simd ) {
    _use_simd = simd;
}

void set_output_file( const char* outfile ) {
    init_out( outfile );
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
void init_score_matrix( const char* matrix_name ) {
    mat_init_buildin( matrix_name );
}

/**
 * Initialises the used scoring matrix from a file. The data
 * is stored internally.
 *
 * @param file_name  path to the file of the matrix
 */
void init_score_matrix_file( const char* file_name ) {
    mat_init_from_file( file_name );
}

/**
 * Initialises the used scoring matrix from a string. The data
 * is stored internally.
 *
 * @param matrix matrix as a string
 */
void init_score_matrix_string( const char* matrix ) {
    mat_init_from_string( matrix );
}

/**
 * Release the memory allocated by function init_score_matrix.
 *
 * @see init_score_matrix
 */
void free_matrix() {
    mat_free();
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
void init_scoring( const int8_t p, const int8_t m ) {
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
    ssa_db_init_fasta( fasta_db_file );

    printf( "DB read %lu sequences\n", ssa_db_get_sequence_count() );
}

/**
 * Release the memory allocated by the function init_db_fasta.
 *
 * @see init_db_fasta
 */
void free_db() {
    ssa_db_free();
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
p_query init_sequence_fasta( const char* fasta_seq_file ) {
    return query_read_from_file( fasta_seq_file );
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

    if( !gapO || !gapE ) {
        ffatal( "Gap opening and/or gap extension cost set to zero. Possible error." );
    }
    if( !score_matrix_63 ) {
        ffatal( "Scoring not initialized." );
    }

    // TODO extend these checks
}

static void prepare_alignment() {
    test_configuration();

    ssa_db_reset_sequence_counter();
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
p_alignment_list sw_align( p_query p, size_t hitcount, int bit_width /* TODO ...*/) {
    prepare_alignment();

    init_for_sw( p, hitcount, bit_width );

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
p_alignment_list nw_align( p_query p, size_t hitcount, int bit_width /* TODO ...*/) {
    prepare_alignment();

    init_for_nw( p, hitcount, bit_width );

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
p_alignment_list nw_sellers_align( p_query p, size_t hitcount, int bit_width /* TODO ...*/) {
    prepare_alignment();

    init_for_nw_sellers( p, hitcount, bit_width );

    return m_run();
}

/**
 * Aligns the query sequence against all sequences in the database using the
 * Needleman-Wunsch Algorithm ignoring gaps in the beginning and/or end.
 *
 * @param  p   pointer to the query profile structure
 * ...
 * @return pointer to the alignment structure
 */
p_alignment_list nw_ignore_gaps_align( p_query p, size_t hitcount, int bit_width /* TODO ... ignored gaps...*/) {
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

    exit_thread_pool();

    close_out();
}
