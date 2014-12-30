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
#include <immintrin.h>

#include "../libssa_datatypes.h"
#include "../util/minheap.h"

struct s16info {
    __m128i matrix[32];
    __m128i * hearray;
    __m128i * dprofile;
    __m128i ** qtable;

    int qlen;
    int maxqlen;
    int maxdlen;

    int16_t penalty_gap_open;
    int16_t penalty_gap_extension;
};

typedef struct s16info * p_s16info;

p_s16info search16_init( int16_t penalty_gap_open, int16_t penalty_gap_extension );

void search16_exit( p_s16info s );

void search16_init_query( p_s16info s, char * qseq, int qlen );

void search16( p_s16info s, p_db_chunk chunk, p_minheap heap, int query_id );
