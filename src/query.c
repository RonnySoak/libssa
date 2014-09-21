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

extern char* sdb_u_revcompl(char* seq, long len);
extern void sdb_u_translate_sequence(char * dna, long dlen, long strand,
        long frame, char ** protp, long * plenp);

extern const char map_ncbi_aa[256];
extern const char map_ncbi_nt16[256];

extern const char * sym_ncbi_nt16;
extern const char * sym_ncbi_aa;

extern int query_strands;
extern int symtype;

static FILE * query_fp;
static char query_line[LINE_MAX];

static p_query init(char * queryname) {
    if (strcmp(queryname, "-") == 0)
        query_fp = stdin;
    else
        query_fp = fopen(queryname, "r");

    if (!query_fp)
        ffatal("Cannot open query file.");

    p_query query = (p_query)xmalloc(sizeof(struct _query));

    query->description = 0;
    query->dlen = 0;

    if ((symtype == AMINOACID) || (symtype == TRANS_DB)) {
        query->map = map_ncbi_aa;
        query->sym = sym_ncbi_aa;
    }
    else {
        query->map = map_ncbi_nt16;
        query->sym = sym_ncbi_nt16;
    }

    for (long s = 0; s < 2; s++) {
        query->nt[s].seq = 0;
        query->nt[s].len = 0;

        for (long f = 0; f < 3; f++) {
            query->aa[3 * s + f].seq = 0;
            query->aa[3 * s + f].len = 0;
        }
    }

    query_line[0] = 0;
    if ( NULL == fgets(query_line, LINE_MAX, query_fp)) {
        ffatal("Could not initialise from query sequence");
    }

    return query;
}

void query_free(p_query query) {
    if (!query)
        return;

    if (query->description)
        free(query->description);
    query->description = 0;
    query->dlen = 0;

    for (long s = 0; s < 2; s++) {
        if (query->nt[s].seq)
            free(query->nt[s].seq);
        query->nt[s].seq = 0;
        query->nt[s].len = 0;

        for (long f = 0; f < 3; f++) {
            if (query->aa[3 * s + f].seq)
                free(query->aa[3 * s + f].seq);
            query->aa[3 * s + f].seq = 0;
            query->aa[3 * s + f].len = 0;
        }
    }

    free(query);
    query = 0;
}

p_query query_read(char * queryname) {
    // open the file and initialise the query
    p_query query = init(queryname);

    // read description
    int len = strlen(query_line);

    if (query_line[len - 1] == '\n') {
        query_line[len - 1] = 0;
        len--;
    }

    if (query_line[0] == '>') {
        query->description = (char*) xmalloc(len);
        strcpy(query->description, query_line + 1);
        query->dlen = len - 1;

        query_line[0] = 0;
        if ( NULL == fgets(query_line, LINE_MAX, query_fp)) {
            ffatal("Could not read first line from query sequence");
        }
    }
    else {
        query->description = (char*) xmalloc(1);
        query->description[0] = 0;
        query->dlen = 0;
    }

    int size = LINE_MAX;
    char * query_sequence = (char *) xmalloc(size);
    query_sequence[0] = 0;
    long query_length = 0;

    char m;
    while (query_line[0] && (query_line[0] != '>')) {
        char * p = query_line;
        int8_t c = *p++;
        while (c) {
            if ((m = query->map[c]) >= 0) {
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

    if ((symtype == NUCLEOTIDE) || (symtype == TRANS_QUERY)
            || (symtype == TRANS_BOTH)) {
        query->nt[0].seq = query_sequence;
        query->nt[0].len = query_length;

        if (query_strands & 2) {
            //      outf("Reverse complement.\n");
            query->nt[1].seq = sdb_u_revcompl(query->nt[0].seq, query->nt[0].len);
            query->nt[1].len = query->nt[0].len;
        }

        if ((symtype == TRANS_QUERY) || (symtype == TRANS_BOTH)) {
            for (long s = 0; s < 2; s++) {
                if ((s + 1) & query_strands) {
                    for (long f = 0; f < 3; f++) {
                        // it always takes the first sequence and reverses it, if necessary, in the translate method
                        sdb_u_translate_sequence(query->nt[0].seq, // dna
                                query->nt[0].len,   // dlen
                                s, // strand
                                f, // frame
                                &query->aa[3 * s + f].seq,  // protp
                                &query->aa[3 * s + f].len); // plenp
                    }
                }
            }
        }
    }
    else {
        query->aa[0].seq = query_sequence;
        query->aa[0].len = query_length;
    }

    // close the file pointer
    if (query_fp != stdin)
        fclose(query_fp);

    return query;
}

void query_show(p_query query) {
    int linewidth = 60;

    if (query->description) {
        for (unsigned i = 0; i < strlen(query->description); i += linewidth) {
            printf("x%i", i);
            if (i == 0)
                outf("Query description: %-60.60s\n", query->description + i);
            else
                outf("                   %-60.60s\n", query->description + i);
        }
    }
    else {
        outf("Query description: UNKNOWN\n");
    }

#if 0
    long qlen;
    char * qseq;
    if ((symtype == NUCLEOTIDE) || (symtype == TRANS_QUERY) || (symtype == TRANS_BOTH)) {
        qseq = query->nt[0].seq;
        qlen = query->nt[0].len;
    }
    else {
        qseq = query->aa[0].seq;
        qlen = query->aa[0].len;
    }

    for (int j=0; j<qlen; j+=linewidth) {
        if (j==0)
        outf( "Query sequence:    ");
        else
        outf( "                   ");

        for(int k=0; k<linewidth; k++) {
            if (j+k < qlen)
            putc(query->sym[qseq[j+k]], out); // TODO out?!
            else
            break;
        }
        outf( "\n");
    }
#endif
}

