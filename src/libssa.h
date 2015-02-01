/*
 * libssa.h
 *
 * LIBSSA - Library for SIMD accelerated optimal Sequence Alignments
 *
 *  Created on: 28 Aug 2014
 *      Author: Jakob Frielingsdorf
 */
#ifndef LIBSSA_H_
#define LIBSSA_H_

#include <stdint.h>

/*
 *
 * TODO do SWIPE and SWARM prepare the DB and query or do we read them from a
 * standard FASTA/BLAST file?
 *
 * banded alignments: do we provide separate functions for these, or
 *      do we add some init-function or parameters for these?
 *
 */

// #############################################################################
// Constants
// #########
#define VERBOSE_ON 1
#define VERBOSE_OFF 0

#define SIMD_ON 1
#define SIMD_OFF 0

#define BLOSUM45 "blosum45"
#define BLOSUM50 "blosum50"
#define BLOSUM62 "blosum62"
#define BLOSUM80 "blosum80"
#define BLOSUM90 "blosum90"
#define PAM30 "pam30"
#define PAM70 "pam70"
#define PAM250 "pam250"

#define NUCLEOTIDE 0 // compare a nucleotide sequence against a nucleotide DB
#define AMINOACID 1 // compare a protein sequence against a protein DB
#define TRANS_QUERY 2 // compare the nucleotide 6-frame translation against a protein DB
#define TRANS_DB 3 // compare a protein sequence against a nucleotide 6-frame translation DB
#define TRANS_BOTH 4 // compare the nucleotide 6-frame translation against a nucleotide 6-frame translation DB

#define FORWARD_STRAND 1 // use only the provided nucleic strand
#define COMPLEMENTARY_STRAND 2 // uses both strands, but translates only the complementary one
#define BOTH_STRANDS 3 // uses both strands and translates both to amino acids

#define BIT_WIDTH_8 8
#define BIT_WIDTH_16 16
#define BIT_WIDTH_64 64

// #############################################################################
// Data types
// ##########

/** @typedef    structure of the query profile  */
struct _query;
typedef struct _query* p_query;

/** @typedef    structure of the alignment result
 *
 *  @field  score1  the best alignment score
 *  @field  score2  sub-optimal alignment score
 *
 *  TODO How do we return the alignments? like SSW or different?
 *  cigar: http://genome.sph.umich.edu/wiki/SAM#What_is_a_CIGAR.3F
 *
 *  SSW:
 @field  ref_begin1  0-based best alignment beginning position on reference;
 ref_begin1 = -1 when the best alignment beginning
 position is not available
 @field  ref_end1    0-based best alignment ending position on reference
 @field  read_begin1 0-based best alignment beginning position on read;
 read_begin1 = -1 when the best alignment beginning
 position is not available
 @field  read_end1   0-based best alignment ending position on read
 @field  read_end2   0-based sub-optimal alignment ending position on read
 @field  cigar   best alignment cigar; stored the same as that in BAM format,
 high 28 bits: length, low 4 bits: M/I/D (0/1/2);
 cigar = 0 when the best alignment path is not available
 @field  cigarLen    length of the cigar string; cigarLen = 0 when the best
 alignment path is not available
 */
//struct {
//    uint16_t score1;
//    uint16_t score2;
//    int32_t ref_begin1;   TODO
//    int32_t ref_end1;
//    int32_t read_begin1;
//    int32_t read_end1;
//    int32_t ref_end2;
//    uint32_t* cigar;
//    int32_t cigarLen;
//} alignment;
//typedef alignment_t* p_alignment;

typedef struct {
    char* seq;
    unsigned long len;
    char* header;
    unsigned long headerlen;
    unsigned long ID;
    int strand;
    int frame;
} db_seq;

typedef struct {
    char* seq;
    unsigned long len;
    int strand;
    int frame;
} q_seq;

typedef struct {
    db_seq db_seq;
    q_seq query;
    char * alignment;
    long alignment_len;
    long score;
//    long score_align;
//    long align_hint;  TODO what are these
//    long bestq;
    long align_q_start;
    long align_q_end;
    long align_d_start;
    long align_d_end;
} alignment_t;
typedef alignment_t * alignment_p; // TODO rename in p_alignment

struct alignment_list {
    alignment_p* alignments;
    long len;
};
typedef struct alignment_list * p_alignment_list;

// #############################################################################
// Configuration data
// ##################
extern int _verbose;
extern long _max_thread_count;
extern int _use_simd;

// #############################################################################
// Technical initialisation
// ########################
void set_verbose(int verbose);

void set_threads(long nr);

void set_use_simd(int simd);

void set_output_file(const char* outfile);

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
void init_score_matrix(const char* matrix_name);

/**
 * Initialises the used scoring matrix from a file. The data
 * is stored internally.
 *
 * @param file_name  path to the file of the matrix
 */
void init_score_matrix_file(const char* file_name);

/**
 * Initialises the used scoring matrix from a string. The data
 * is stored internally.
 *
 * @param matrix  matrix as a string
 */
void init_score_matrix_string(const char* matrix);

/**
 * Release the memory allocated by function init_score_matrix.
 *
 * @see init_score_matrix
 */
void free_matrix();

/**
 * Initialises gap penalties used for the alignments.
 *
 * @param  gapO  penalty for opening a gap
 * @param  gapE  penalty for extending a gap
 */
void init_gap_penalties(const uint8_t gapO, const uint8_t gapE);

/**
 * Initialises the scoring scheme if no scoring matrix is used.
 *
 * Should only be used, if no scoring matrix is used.
 *
 * @param  p    penalty for a mismatch
 * @param  m    reward for a match
 */
void init_scoring(const int8_t p, const int8_t m);

/**
 * Initialises the symbol type translation for the alignment. Depending on the
 * symbol type the query and/or database sequences are translated, prior to the
 * alignment.
 *
 * Available types are:
 *  - NUCLEOTIDE = 0
 *  - AMINOACID = 1
 *  - TRANS_QUERY = 2
 *  - TRANS_DB = 3
 *  - TRANS_BOTH = 4
 *
 * Default is AMINOACID.
 *
 * Possible values for strands are:
 *  - FORWARD_STRAND
 *  - COMPLEMENTARY_STRAND
 *  - BOTH_STRANDS
 *
 * Possible values for the genetic codes of DB and query: [1-23]
 * TODO add list of codes
 */
void init_symbol_translation(int type, int strands, int db_gencode,
        int q_gencode);

/**
 * Reads a FASTA file containing multiple sequences to compare the query
 * sequence against.
 * The initialised data is stored internally.
 *
 * @param fasta_db_file  path to a file in FASTA format
 */
void init_db_fasta(const char* fasta_db_file);

/**
 * Initialise the database lib, to use a third party implementation.
 */
//void sdb_init_external(p_sdb_sequence (*extern_next_sequence)()); TODO make db lib configurable
// TODO possibly best via linking or crating a dynamic lib

/**
 * Release the memory allocated by the function init_db_fasta.
 *
 * @see init_db_fasta
 */
void free_db();

/**
 * Reads a FASTA file containing the query sequence.
 *
 * @param fasta_seq_file  path to a file in FASTA format
 * @return pointer to the query profile structure
 */
p_query init_sequence_fasta(const char* fasta_seq_file);

/**
 * Release the memory allocated by the function init_sequence_fasta.
 *
 * @param  p   pointer to the query profile structure
 *
 * @see init_sequence_fasta
 */
void free_sequence(p_query p);

// #############################################################################
// Alignment
// #########
/**
 * Aligns the query sequence against all sequences in the database using the
 * Smith-Waterman Algorithm.
 *
 * @param  p   pointer to the query profile structure
 * ...
 * @return pointer to the alignment structure
 */
p_alignment_list sw_align(p_query p, int hitcount, int bit_width /* TODO ...*/);

/**
 * Aligns the query sequence against all sequences in the database using the
 * Needleman-Wunsch Algorithm.
 *
 * @param  p   pointer to the query profile structure
 * ...
 * @return pointer to the alignment structure
 */
p_alignment_list nw_align(p_query p, int hitcount, int bit_width /* TODO ...*/);

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
p_alignment_list nw_sellers_align(p_query p, int hitcount, int bit_width /* TODO ...*/);

/**
 * Aligns the query sequence against all sequences in the database using the
 * Needleman-Wunsch Algorithm ignoring gaps in the beginning and/or end.
 *
 * @param  p   pointer to the query profile structure
 * ...
 * @return pointer to the alignment structure
 */
p_alignment_list nw_ignore_gaps_align(p_query p, int hitcount, int bit_width /* TODO ... ignored gaps...*/);

/**
 * Release the memory allocated by the functions sw_align, nw_align,
 * nw_sellers_align and nw_ignore_gaps_align.
 *
 * @param  a   pointer to the alignment structure
 *
 * @see sw_align
 * @see nw_align
 */
void free_alignment(p_alignment_list alist);

void ssa_exit();

#endif /* LIBSSA_H_ */
