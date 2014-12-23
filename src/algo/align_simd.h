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

#include "../libssa_datatypes.h"

struct s16info_s;

struct s16info_s * search16_init( int16_t penalty_gap_open, int16_t penalty_gap_extension );

void search16_exit( struct s16info_s * s );

void search16_qprep( struct s16info_s * s, char * qseq, int qlen );

void search16( struct s16info_s * s, p_db_chunk chunk, int16_t * pscores, unsigned short * paligned,
        unsigned short * pmatches, unsigned short * pmismatches, unsigned short * pgaps, char ** pcigar );
