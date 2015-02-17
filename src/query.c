/*
 * query.c
 *
 *  Created on: Sep 3, 2014
 *      Author: Jakob Frielingsdorf
 */

#include "query.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "libssa.h"
#include "util/util_sequence.h"
#include "util/util.h"

int symtype = DEFAULT_SYMTYPE;
int query_strands = DEFAULT_STRAND;

/**
 * Initialises the query struct.
 *
 * @return  a pointer to the newly created query struct
 */
static p_query init() {
    p_query query = xmalloc( sizeof(struct _query) );

    query->header = 0;
    query->headerlen = 0;

    if( (symtype == AMINOACID) || (symtype == TRANS_DB) ) {
        query->map = map_ncbi_aa;
        query->sym = sym_ncbi_aa;
    }
    else {
        query->map = map_ncbi_nt16;
        query->sym = sym_ncbi_nt16;
    }

    for( int s = 0; s < 2; s++ ) {
        query->nt[s].seq = 0;
        query->nt[s].len = 0;

        for( int f = 0; f < 3; f++ ) {
            query->aa[3 * s + f].seq = 0;
            query->aa[3 * s + f].len = 0;
        }
    }

    return query;
}

/**
 * Releases the memory used by the query.
 *
 * @param query     pointer to the query memory
 */
void query_free( p_query query ) {
    if( !query )
        return;

    if( query->header )
        free( query->header );
    query->header = 0;
    query->headerlen = 0;

    for( int s = 0; s < 2; s++ ) {
        if( query->nt[s].seq )
            free( query->nt[s].seq );
        query->nt[s].seq = 0;
        query->nt[s].len = 0;

        for( int f = 0; f < 3; f++ ) {
            if( query->aa[3 * s + f].seq )
                free( query->aa[3 * s + f].seq );
            query->aa[3 * s + f].seq = 0;
            query->aa[3 * s + f].len = 0;
        }
    }

    free( query );
    query = 0;
}

static void fill_query( p_query query, char * query_sequence, size_t query_length ) {
    if( (symtype == NUCLEOTIDE) || (symtype == TRANS_QUERY) || (symtype == TRANS_BOTH) ) {
        query->nt[0] = (sequence ) { query_sequence, query_length };

        if( query_strands & 2 ) {
            query->nt[1] = (sequence ) { xmalloc( query_length + 1 ), query_length };

            us_revcompl( query->nt[0], query->nt[1] );
        }

        if( (symtype == TRANS_QUERY) || (symtype == TRANS_BOTH) ) {
            for( int s = 0; s < 2; s++ ) {
                if( (s + 1) & query_strands ) {
                    for( int f = 0; f < 3; f++ ) {
                        // it always takes the first sequence and reverses it, if necessary, in the translate method
                        us_translate_sequence( 0, // query sequence
                                query->nt[0], // DNA
                                s, // strand
                                f, // frame
                                &query->aa[3 * s + f] ); // Protein
                    }
                }
            }
        }
    }
    else {
        query->aa[0] = (sequence ) { query_sequence, query_length };
    }
}

p_query query_read_from_string( char * header, char * sequence ) {
    p_query query = init();

    query->headerlen = strlen( header );
    query->header = xmalloc( query->headerlen );
    strcpy( query->header, header );

    size_t length = strlen( sequence );
    char * query_sequence = xmalloc( length );

    char m;
    char * p = sequence;
    char * p_q = query_sequence;
    int8_t c = *p++;
    while( c ) {
        if( (m = query->map[c]) >= 0 ) {
            (*p_q++) = m;
        }
        c = *p++;
    }

    query_sequence[length] = 0;

    fill_query( query, query_sequence, length );

    return query;
}

/**
 * Reads in a query from a file and returns a pointer to a struct holding the
 * query data.
 *
 * @param queryname     name of the file containing the query
 * @return              a pointer to the read query
 */
p_query query_read_from_file( const char * filename ) {
    FILE * query_fp = 0;

    if( strcmp( filename, "-" ) == 0 )
        ffatal( "Query not specified" ); // TODO
    else
        query_fp = fopen( filename, "r" );

    if( !query_fp )
        ffatal( "Cannot open query file: %s", filename );

    p_query query = init();

    char query_line[LINE_MAX];

    query_line[0] = 0;
    if( NULL == fgets( query_line, LINE_MAX, query_fp ) ) {
        ffatal( "Could not initialise from query sequence" );
    }

    // read description
    size_t len = strlen( query_line );

    if( query_line[len - 1] == '\n' ) {
        query_line[len - 1] = 0;
        len--;
    }

    if( query_line[0] == '>' ) {
        query->header = xmalloc( len );
        strcpy( query->header, query_line + 1 );
        query->headerlen = len - 1;

        query_line[0] = 0;
        if( NULL == fgets( query_line, LINE_MAX, query_fp ) ) {
            ffatal( "Could not read first line from query sequence" );
        }
    }
    else {
        query->header = xmalloc( 1 );
        query->header[0] = 0;
        query->headerlen = 0;
    }

    // read sequence
    size_t size = LINE_MAX;
    char * query_sequence = xmalloc( size );
    query_sequence[0] = 0;
    size_t query_length = 0;

    char m;
    while( query_line[0] && (query_line[0] != '>') ) {
        char * p = query_line;
        int8_t c = *p++;
        while( c ) {
            if( (m = query->map[c]) >= 0 ) {
                if( query_length + 1 >= size ) {
                    size += LINE_MAX;
                    query_sequence = xrealloc( query_sequence, size );
                }
                query_sequence[query_length++] = m;
            }
            c = *p++;
        }
        query_line[0] = 0;
        if( NULL == fgets( query_line, LINE_MAX, query_fp ) ) {
            break;
        }
    }
    query_sequence[query_length] = 0;

    fill_query( query, query_sequence, query_length );

    // close the file pointer
    if( query_fp && (query_fp != stdin) )
        fclose( query_fp );

    return query;
}

/**
 * Prints the query to the configured output file.
 *
 * @param query     the query data to print
 *
 * TODO not tested
 */
void query_show( p_query query ) {
    int linewidth = 60;

    if( query->header ) {
        for( size_t i = 0; i < query->headerlen; i += linewidth ) {
            outf( "x%ld", i );
            if( i == 0 )
                outf( "Query description: %-60.60s\n", query->header + i );
            else
                outf( "                   %-60.60s\n", query->header + i );
        }
    }
    else {
        outf( "Query description: UNKNOWN\n" );
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

