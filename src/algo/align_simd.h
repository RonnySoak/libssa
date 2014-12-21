/*
 Copyright (C) 2014 Torbjorn Rognes

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

 Contact: Torbjorn Rognes <torognes@ifi.uio.no>,
 Department of Informatics, University of Oslo,
 PO Box 1080 Blindern, NO-0316 Oslo, Norway
 */

#include <stdint.h>

typedef signed short int16_t;

struct s16info_s;

struct s16info_s * search16_init( int16_t score_match, int16_t score_mismatch, int16_t penalty_gap_open_query_left,
        int16_t penalty_gap_open_target_left, int16_t penalty_gap_open_query_interior,
        int16_t penalty_gap_open_target_interior, int16_t penalty_gap_open_query_right,
        int16_t penalty_gap_open_target_right, int16_t penalty_gap_extension_query_left,
        int16_t penalty_gap_extension_target_left, int16_t penalty_gap_extension_query_interior,
        int16_t penalty_gap_extension_target_interior, int16_t penalty_gap_extension_query_right,
        int16_t penalty_gap_extension_target_right );

void search16_exit( struct s16info_s * s );

void search16_qprep( struct s16info_s * s, char * qseq, int qlen );

void search16( struct s16info_s * s, unsigned int sequences, unsigned int * seqnos, int16_t * pscores,
        unsigned short * paligned, unsigned short * pmatches, unsigned short * pmismatches, unsigned short * pgaps,
        char * * pcigar );
