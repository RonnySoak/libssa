/*
 * generate_db.c
 *
 *  Created on: Feb 24, 2015
 *      Author: Jakob Frielingsdorf
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>

const char sym_ncbi_nt16u[16] = "-ACMGRSVTWYHKDBN";
const char sym_ncbi_aa[28] = "-ABCDEFGHIKLMNPQRSTVWXYZU*OJ";

static void print_help() {
    printf("Usage: ./benchmark [options]\n");
    printf("Commandline options:\n");

    printf("  -n            Number of used sequences\n");
    printf("  -m            0: amino acids, 1: nucleotides, or 2: usable for both\n" );
    printf("  -f            Filename for generated database\n" );
    printf("  -a            Max length of the generated sequences\n" );
    printf("  -i            Min length of the generated sequences\n" );
    printf("\n");

    exit(EXIT_FAILURE);
}

static void read_options( int argc, char ** argv, int * mode, int * count, char ** filename, int * max_length, int * min_length ) {
    int c;

    if( argc == 1 ) {
        print_help();
    }

    while( (c = getopt( argc, argv, "n:m:f:a:i:" )) != -1 ) {
        switch( c ) {
        case 'n':
            *count = atoi( optarg );
            break;
        case 'm':
            *mode = atoi( optarg );
            break;
        case 'a':
            *max_length = atoi( optarg );
            break;
        case 'i':
            *min_length = atoi( optarg );
            break;
        case 'f':
            *filename = optarg;
            break;
        default:
            print_help();
            break;
        }
    }

    if( *max_length < *min_length ) {
        printf( "Max-length is shorter than min-length.\n" );
        exit( 1 );
    }

    if( *min_length <= 1 ) {
        printf( "Min-length set to 1!\n" );
        *min_length = 1;
    }
    if( *max_length > UINT16_MAX ) {
        printf( "Max-length set to %d!\n", UINT16_MAX );
        *max_length = UINT16_MAX;
    }
}

int main( int argc, char**argv ) {
    int mode;
    int count;
    char * filename;
    int max_length;
    int min_length;

    read_options( argc, argv, &mode, &count, &filename, &max_length, &min_length );

    FILE *f = fopen( filename, "w" );
    if( f == NULL ) {
        printf( "Error opening file!\n" );
        exit( 1 );
    }

    srand(time(NULL));

    char buffer[max_length + 1];

    for( int i = 0; i < count; ++i ) {
        int length = -1;
        while( length < min_length ) {
            length = rand() % max_length;
        }
        fprintf( f, ">generated sequence: %d; length: %d\n", i, length );

        // generates a random sequence
        for( int x = 0; x < length; ++x ) {
            int r = rand();

            if( mode == 0 ) {
                buffer[x] = sym_ncbi_aa[r % 28];
            }
            else {
                buffer[x] = sym_ncbi_nt16u[r % 16];
            }
        }
        buffer[length] = 0;
        fprintf( f, "%s\n", buffer );
    }

    fclose( f );

    return 0;
}
