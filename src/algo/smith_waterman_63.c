/*
 SWIPE
 Smith-Waterman database searches with Inter-sequence Parallel Execution

 Copyright (C) 2008-2012 Torbjorn Rognes, University of Oslo,
 Oslo University Hospital and Sencel Bioinformatics AS

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

#include "search.h"

#include <string.h>

#include "../util.h"
#include "../matrices.h"

int64_t full_sw(sequence * dseq, sequence * qseq, int64_t * hearray,
        int64_t * score_matrix) {

    int64_t h, n, e, f, s;
    int64_t *hep;
    const char *qp, *dp;
    const int64_t * sp;

    char* qend = qseq->seq + qseq->len;
    char* dend = dseq->seq + dseq->len;

    s = 0;
    dp = dseq->seq;
    memset(hearray, 0, 2 * sizeof(int64_t) * (qend - qseq->seq));

    while (dp < dend) {
        f = 0;
        h = 0;
        hep = hearray;
        qp = qseq->seq;
        sp = score_matrix + (*dp << 5);

        while (qp < qend) {
            n = *hep;
            e = *(hep + 1);
            h += sp[(int) (*qp)];

            if (e > h)
                h = e;
            if (f > h)
                h = f;
            if (h < 0)
                h = 0;
            if (h > s)
                s = h;

            *hep = h;
            e -= gapE;
            f -= gapE;
            h -= gapO - gapE;

            if (h > e)
                e = h;
            if (h > f)
                f = h;

            *(hep + 1) = e;
            h = n;
            hep += 2;
            qp++;
        }

        dp++;
    }

    return s;
}

