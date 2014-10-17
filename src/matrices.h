/*
 * matrices.h
 *
 *  Created on: Oct 12, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef MATRICES_H_
#define MATRICES_H_

#include <stddef.h>
#include <stdint.h>

#define SCORE_MATRIX_7(x, y) (score_matrix_7[(x << 5) + y])
#define SCORE_MATRIX_8(x, y) (score_matrix_8[(x << 5) + y])
#define SCORE_MATRIX_16(x, y) (score_matrix_16[(x << 5) + y])
#define SCORE_MATRIX_32(x, y) (score_matrix_32[(x << 5) + y])
#define SCORE_MATRIX_63(x, y) (score_matrix_63[(x << 5) + y])

extern const char mat_blosum45[];
extern const char mat_blosum50[];
extern const char mat_blosum62[];
extern const char mat_blosum80[];
extern const char mat_blosum90[];

extern const char mat_pam30[];
extern const char mat_pam70[];
extern const char mat_pam250[];

extern int8_t* score_matrix_7;
extern uint8_t * score_matrix_8;
extern int16_t * score_matrix_16;
extern uint32_t * score_matrix_32;
extern int64_t * score_matrix_63;

/**
 * Prints the currently initialised scoring matrix to the specified output file.
 *
 * @param outfile   the file, where to print the matrix
 */
void mat_dump(char* outfile);

/**
 * Initialises matrices with a constant score for matches and mismatches.
 *
 * Constant scores can be used with DNA and protein sequences.
 *
 * @param matchscore        score for matching symbols
 * @param mismatchscore     score for mismatching symbols
 */
void mat_init_constant_scoring(const int8_t matchscore,
        const int8_t mismatchscore);

/**
 * Reads a scoring matrix from a file.
 *
 * @param matrix    name of the file containing the scoring matrix
 */
void mat_init_from_file(const char * matrix);

/**
 * Reads a scoring matrix from a string.
 *
 * @param matrix    scoring matrix as a string
 */
void mat_init_from_string(const char * matrix);

/**
 * Prepares one of the build-in scoring matrixes.
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
 *
 * @param matrixname    one of the available matrices
 */
void mat_init_buildin(const char* matrixname);

/**
 * Releases the memory allocated for the scoring matrices.
 */
void mat_free();


#endif /* MATRICES_H_ */
