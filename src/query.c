/*
 * query.c
 *
 *  Created on: Sep 3, 2014
 *      Author: Jakob Frielingsdorf
 */

#include <string.h>

#include "util.h"
#include "libssa_datatypes.h"
#include "libssa.h"

//   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
//   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   |
char map_ncbi_aa[256] =
        {
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25, -1, -1, 0, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 27, 10, 11, 12, 13, 26,
                14, 15, 16, 17, 18, 24, 19, 20, 21, 22, 23, -1, -1, -1, -1, -1,
                -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 27, 10, 11, 12, 13, 26,
                14, 15, 16, 17, 18, 24, 19, 20, 21, 22, 23, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        };

char map_ncbi_nt4[256] =
        {
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, 0, -1, 1, -1, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, 3, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, 0, -1, 1, -1, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, 3, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        };

char map_ncbi_nt16[256] =
        {
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, 1, 14, 2, 13, -1, -1, 4, 11, -1, -1, 12, -1, 3, 15, -1,
                -1, -1, 5, 6, 8, 8, 7, 9, -1, 10, -1, -1, -1, -1, -1, -1,
                -1, 1, 14, 2, 13, -1, -1, 4, 11, -1, -1, 12, -1, 3, 15, -1,
                -1, -1, 5, 6, 8, 8, 7, 9, -1, 10, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        };

char ntcompl[16] = { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };

char q_translate[16 * 16 * 16];
char d_translate[16 * 16 * 16];

char * code[23] =
        {
                "FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSS**VVVVAAAADDEEGGGG",
                "FFLLSSSSYY**CCWWTTTTPPPPHHQQRRRRIIMMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSSSSVVVVAAAADDEEGGGG",
                "FFLLSSSSYYQQCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                NULL,
                NULL,
                "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNNKSSSSVVVVAAAADDEEGGGG",
                "FFLLSSSSYY**CCCWLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                "FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                "FFLLSSSSYY**CC*WLLLSPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSSGGVVVVAAAADDEEGGGG",
                "FFLLSSSSYYY*CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNNKSSSSVVVVAAAADDEEGGGG",
                "FFLLSSSSYY*QCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                "FFLLSSSSYY*LCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                NULL,
                NULL,
                NULL,
                NULL,
                "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNNKSSSSVVVVAAAADDEEGGGG",
                "FFLLSS*SYY*LCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
                "FF*LSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG"
        };

char remap[] = { 2, 1, 3, 0 };

struct _query query;

//                       00000000001111111111222222222233
//                       01234567890123456789012345678901
const char * sym_ncbi_nt4 = "acgt############################";
const char * sym_ncbi_nt16 = "-acmgrsvtwyhkdbn################";
const char * sym_ncbi_nt16u = "-ACMGRSVTWYHKDBN################";
const char * sym_ncbi_aa = "-ABCDEFGHIKLMNPQRSTVWXYZU*OJ####";

FILE * query_fp;
char query_line[LINE_MAX];

char * revcompl(char * seq, long len) {
    char * rc = (char *) xmalloc(len + 1);
    for (long i = 0; i < len; i++)
        rc[i] = ntcompl[(int) (seq[len - 1 - i])];
    rc[len] = 0;
    return rc;
}

void translate(char * dna, long dlen,
        long strand, long frame, long table,
        char ** protp, long * plenp) {
    //  outf("dlen=%ld, strand=%ld, frame=%ld\n", dlen, strand, frame);

    char * ttable;
    if (table == 0)
        ttable = q_translate;
    else
        ttable = d_translate;

    long pos, c;
    long ppos = 0;
    long plen = (dlen - frame) / 3;
    char * prot = (char*) xmalloc(1 + plen);

    if (strand == 0) {
        pos = frame;
        while (ppos < plen) {
            c = dna[pos++];
            c <<= 4;
            c |= dna[pos++];
            c <<= 4;
            c |= dna[pos++];
            prot[ppos++] = ttable[c];
        }
    }
    else {
        pos = dlen - 1 - frame;
        while (ppos < plen) {
            c = ntcompl[(int) (dna[pos--])];
            c <<= 4;
            c |= ntcompl[(int) (dna[pos--])];
            c <<= 4;
            c |= ntcompl[(int) (dna[pos--])];
            prot[ppos++] = ttable[c];
        }
    }

    prot[ppos] = 0;
    *protp = prot;
    *plenp = plen;
}

void translate_createtable(long tableno, char * table) {
    /* initialise translation table */

    for (long a = 0; a < 16; a++)
        for (long b = 0; b < 16; b++)
            for (long c = 0; c < 16; c++) {
                char aa = '-';
                for (long i = 0; i < 4; i++)
                    for (long j = 0; j < 4; j++)
                        for (long k = 0; k < 4; k++)
                                {
                            if ((a & (1 << i)) && (b & (1 << j))
                                    && (c & (1 << k)))
                                    {
                                long codon = remap[i] * 16 + remap[j] * 4
                                        + remap[k];
                                char x = code[tableno - 1][codon];
                                if (aa == '-') {
                                    aa = x;
                                }
                                else if (aa == x) {
                                }
                                else if ((aa == 'B')
                                        && ((x == 'D') || (x == 'N'))) {
                                }
                                else if ((aa == 'D')
                                        && ((x == 'B') || (x == 'N'))) {
                                    aa = 'B';
                                }
                                else if ((aa == 'N')
                                        && ((x == 'B') || (x == 'D')))
                                        {
                                    aa = 'B';
                                }
                                else if ((aa == 'Z')
                                        && ((x == 'Q') || (x == 'E'))) {
                                }
                                else if ((aa == 'E')
                                        && ((x == 'Z') || (x == 'Q'))) {
                                    aa = 'Z';
                                }
                                else if ((aa == 'Q')
                                        && ((x == 'Z') || (x == 'E'))) {
                                    aa = 'Z';
                                }
                                else {
                                    aa = 'X';
                                }
                            }
                        }

                if (aa == '-')
                    aa = 'X';

                table[256 * a + 16 * b + c] = map_ncbi_aa[(int) aa];
            }

#if 0
    /* dump it */

    outf("          -ACMGRSVTWYHKDBN\n");
    for(long x=0; x<16; x++)
    for(long y=0; y<16; y++) {
        outf("%2ld %2ld %c %c ", x, y, sym_ncbi_nt16[x], sym_ncbi_nt16[y]);
        for(long z=0; z<16; z++) {
            outf("%c", sym_ncbi_aa[table[256*x+16*y+z]]);
        }
        outf("\n");
    }
#endif
}

void translate_init(long qtableno, long dtableno) {
    translate_createtable(qtableno, q_translate);
    translate_createtable(dtableno, d_translate);
}

void query_init(char * queryname, long strands) {
    if (strcmp(queryname, "-") == 0)
        query_fp = stdin;
    else
        query_fp = fopen(queryname, "r");

    if (!query_fp)
        ffatal("Cannot open query file.");

    query.description = 0;
    query.dlen = 0;
    query.strands = strands;

    if ((symtype == AMINOACID) || (symtype == TRANS_DB)) {
        query.map = map_ncbi_aa;
        query.sym = sym_ncbi_aa;
    }
    else {
        query.map = map_ncbi_nt16;
        query.sym = sym_ncbi_nt16;
    }

    for (long s = 0; s < 2; s++) {
        query.nt[s].seq = 0;
        query.nt[s].len = 0;

        for (long f = 0; f < 3; f++) {
            query.aa[3 * s + f].seq = 0;
            query.aa[3 * s + f].len = 0;
        }
    }

    query_line[0] = 0;
    if ( NULL == fgets(query_line, LINE_MAX, query_fp)) {
        ffatal("Could not initialise from query sequence");
    }
}

void query_free() {
    if (query.description)
        free(query.description);
    query.description = 0;
    query.dlen = 0;

    for (long s = 0; s < 2; s++) {
        if (query.nt[s].seq)
            free(query.nt[s].seq);
        query.nt[s].seq = 0;
        query.nt[s].len = 0;

        for (long f = 0; f < 3; f++) {
            if (query.aa[3 * s + f].seq)
                free(query.aa[3 * s + f].seq);
            query.aa[3 * s + f].seq = 0;
            query.aa[3 * s + f].len = 0;
        }
    }
}

int query_read() {
    if (!query_line[0])
        return 0;

    query_free();

    // read description

    int len = strlen(query_line);

    if (query_line[len - 1] == '\n') {
        query_line[len - 1] = 0;
        len--;
    }

    if (query_line[0] == '>') {
        query.description = (char*) xmalloc(len);
        strcpy(query.description, query_line + 1);
        query.dlen = len - 1;

        query_line[0] = 0;
        if ( NULL == fgets(query_line, LINE_MAX, query_fp)) {
            ffatal("Could not read first line from query sequence");
        }
    }
    else {
        query.description = (char*) xmalloc(1);
        query.description[0] = 0;
        query.dlen = 0;
    }

    int size = LINE_MAX;
    char * query_sequence = (char *) xmalloc(size);
    query_sequence[0] = 0;
    long query_length = 0;

    char * map;
    char m;

    if ((symtype == AMINOACID) || (symtype == TRANS_DB))
        map = map_ncbi_aa;
    else
        map = map_ncbi_nt16;

    while (query_line[0] && (query_line[0] != '>')) {
        char * p = query_line;
        int8_t c = *p++;
        while (c) {
            if ((m = map[c]) >= 0) {
                if (query_length + 1 >= size) {
                    size += LINE_MAX;
                    query_sequence = (char*) xrealloc(query_sequence, size);
                }
                query_sequence[query_length++] = m;
            }
            c = *p++;
        }
        query_line[0] = 0;
        if ( NULL == fgets(query_line, LINE_MAX, query_fp)) {
            break;
        }
    }
    query_sequence[query_length] = 0;

    if ((symtype == NUCLEOTIDE) || (symtype == TRANS_QUERY) || (symtype == TRANS_BOTH)) {
        query.nt[0].seq = query_sequence;
        query.nt[0].len = query_length;

        if (query.strands & 2) {
            //      outf("Reverse complement.\n");
            query.nt[1].seq = revcompl(query.nt[0].seq, query.nt[0].len);
            query.nt[1].len = query.nt[0].len;
        }

        if ((symtype == TRANS_QUERY) || (symtype == TRANS_BOTH)) {
            for (long s = 0; s < 2; s++) {
                if ((s + 1) & query.strands) {
                    for (long f = 0; f < 3; f++) {
                        translate(query.nt[0].seq, query.nt[0].len, s, f, 0,
                                &query.aa[3 * s + f].seq,
                                &query.aa[3 * s + f].len);
                    }
                }
            }
        }
    }
    else {
        query.aa[0].seq = query_sequence;
        query.aa[0].len = query_length;
    }

    return 1;
}

void query_exit() {
    if (query_fp != stdin)
        fclose(query_fp);

    query_free();
}

void query_show() {
    int linewidth = 60;

    if(query.description) {
        for (unsigned i = 0; i < strlen(query.description); i += linewidth) {
            printf("x%i", i);
            if (i == 0)
                outf("Query description: %-60.60s\n", query.description + i);
            else
                outf("                   %-60.60s\n", query.description + i);
        }
    }
    else {
        outf("Query description: UNKNOWN\n");
    }

#if 0
    long qlen;
    char * qseq;
    if ((symtype == NUCLEOTIDE) || (symtype == TRANS_QUERY) || (symtype == TRANS_BOTH)) {
        qseq = query.nt[0].seq;
        qlen = query.nt[0].len;
    }
    else {
        qseq = query.aa[0].seq;
        qlen = query.aa[0].len;
    }

    for (int j=0; j<qlen; j+=linewidth) {
        if (j==0)
        outf( "Query sequence:    ");
        else
        outf( "                   ");

        for(int k=0; k<linewidth; k++) {
            if (j+k < qlen)
            putc(query.sym[qseq[j+k]], out); // TODO out?!
            else
            break;
        }
        outf( "\n");
    }
#endif
}

