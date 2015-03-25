/*
 Copyright (C) 2014-2015 Jakob Frielingsdorf

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

 Contact: Jakob Frielingsdorf <jfrielingsdorf@gmail.com>
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
    p_query query = xmalloc( sizeof(query_t) );

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

static void map_sequence( const char * orig, sequence_t * mapped, p_query query ) {
    char * unknown_symbols = xmalloc( mapped->len );
    size_t unknown_count = 0;

    size_t added_symbol_count = 0;

    char m;
    for( int i = 0; i < mapped->len; ++i ) {
        if( (m = query->map[(int )orig[i]]) >= 0 ) {
            mapped->seq[added_symbol_count++] = m;
        }
        else {
            if( orig[i] != '\n' && orig[i] != ' ' && orig[i] != '\t' ) {
                unknown_symbols[unknown_count++] = orig[i];
            }
        }
    }
    if( unknown_count > 0 ) {
        unknown_symbols[unknown_count] = 0;

        outf( "Unknown symbols found and omitted: %ld - '%s'\n", unknown_count, unknown_symbols );
    }
    free( unknown_symbols );

    mapped->seq[added_symbol_count] = 0;

    mapped->len = added_symbol_count;

    mapped->seq = xrealloc( mapped->seq, mapped->len + 1 );
}

static void fill_and_map_query( p_query query, char * query_sequence, size_t query_length ) {
    sequence_t orig = (sequence_t ) { xmalloc( query_length + 1 ), query_length };

    map_sequence( query_sequence, &orig, query );

    if( (symtype == NUCLEOTIDE) || (symtype == TRANS_QUERY) || (symtype == TRANS_BOTH) ) {
        query->nt[0] = orig;

        if( query_strands & 2 ) {
            query->nt[1] = (sequence_t ) { xmalloc( query_length + 1 ), query_length };

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
        query->aa[0] = orig;
    }
}

p_query query_read_from_string( const char * sequence ) {
    p_query query = init();

    size_t len = strlen( sequence );

    char * query_sequence = xmalloc( len );
    strncpy( query_sequence, sequence, len );

    fill_and_map_query( query, query_sequence, len );

    free( query_sequence );

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

    // read sequence
    size_t size = LINE_MAX;
    char * query_sequence = xmalloc( size );

    size_t query_length = 0;

    while( query_line[0] && (query_line[0] != '>') ) {

        for( int i = 0; query_line[i]; ++i ) {
            if( query_length + 1 >= size ) {
                size += LINE_MAX;
                query_sequence = xrealloc( query_sequence, size );
            }
            query_sequence[query_length++] = query_line[i];
        }

        if( NULL == fgets( query_line, LINE_MAX, query_fp ) ) {
            break;
        }
    }
    query_sequence[query_length] = 0;

    fill_and_map_query( query, query_sequence, query_length );
    free( query_sequence );

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

