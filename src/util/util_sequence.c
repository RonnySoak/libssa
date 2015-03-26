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

#include <stdlib.h>

#include "util_sequence.h"
#include "util.h"
#include "../query.h" // TODO remove (move symtype/strands out of query.h file)

/*
 * Maps amino acids to their numerical representation. Maps upper case and lower
 * case letters.
 *
 * The supported symbols are:
 * 0                          27
 * -ABCDEFGHIKLMNPQRSTVWXYZU*OJ
 */
const char map_ncbi_aa[256] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 25, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 27, 10, 11, 12, 13, 26,
  14, 15, 16, 17, 18, 24, 19, 20, 21, 22, 23, -1, -1, -1, -1, -1,
  -1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 27, 10, 11, 12, 13, 26,
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

/*
 * Maps nucleotides to their numerical representation. Maps upper case and lower
 * case letters.
 *
 * The supported symbols are:
 * 0              16
 * -ACMGRSVTWYHKDBN
 */
const char map_ncbi_nt16[256] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1,  1, 14,  2, 13, -1, -1,  4, 11, -1, -1, 12, -1,  3, 15, -1,
  -1, -1,  5,  6,  8,  8,  7,  9, -1, 10, -1, -1, -1, -1, -1, -1,
  -1,  1, 14,  2, 13, -1, -1,  4, 11, -1, -1, 12, -1,  3, 15, -1,
  -1, -1,  5,  6,  8,  8,  7,  9, -1, 10, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};


// TODO provide to the user of the lib
/*
 * Descriptions of the genetic codes, available for translating genetic sequences
 * to protein sequences.
 */
const char * gencode_names[25] = {
        "Standard Code",
        "Vertebrate Mitochondrial Code",
        "Yeast Mitochondrial Code",
        "Mold, Protozoan, and Coelenterate Mitochondrial Code and Mycoplasma/Spiroplasma Code",
        "Invertebrate Mitochondrial Code",
        "Ciliate, Dasycladacean and Hexamita Nuclear Code",
        NULL,
        NULL,
        "Echinoderm and Flatworm Mitochondrial Code",
        "Euplotid Nuclear Code",
        "Bacterial, Archaeal and Plant Plastid Code",
        "Alternative Yeast Nuclear Code",
        "Ascidian Mitochondrial Code",
        "Alternative Flatworm Mitochondrial Code",
        "Blepharisma Nuclear Code",
        "Chlorophycean Mitochondrial Code",
        NULL,
        NULL,
        NULL,
        NULL,
        "Trematode Mitochondrial Code",
        "Scenedesmus obliquus Mitochondrial Code",
        "Thraustochytrium Mitochondrial Code",
        "Pterobranchia Mitochondrial Code",
        "Candidate Division SR1 and Gracilibacteria Code" };

/*
 * List of genetic codes, for translating genetic sequences to protein sequences.
 */
static const char * code[25] = {
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
        "FF*LSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG",
        "FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSSKVVVVAAAADDEEGGGG",
        "FFLLSSSSYY**CCGWLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG" };

/*
 * Maps the numerical representation of nucleotides back to symbols in lower case.
 * Maps 16 codes for nucleotides:
 *
 * -acmgrsvtwyhkdbn
 */
const char * sym_ncbi_nt16 = "-acmgrsvtwyhkdbn################";
/*
 * Maps the numerical representation of nucleotides back to symbols in upper case.
 * Maps 16 codes for nucleotides:
 *
 * -ACMGRSVTWYHKDBN
 */
const char * sym_ncbi_nt16u = "-ACMGRSVTWYHKDBN################";
/*
 * Maps the numerical representation of amino acids back to symbols in upper case.
 * Maps 28 codes for amino acids:
 *
 * -ABCDEFGHIKLMNPQRSTVWXYZU*OJ
 */
const char * sym_ncbi_aa = "-ABCDEFGHIKLMNPQRSTVWXYZU*OJ####";
/*
 * Contains the numerical representation of the complementary nucleotide symbol.
 */
static const char ntcompl[16] = { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };

/*
 * Table for translating a nucleic query sequence into a protein sequence.
 * Initialized with the genetic code of the query sequence.
 */
static char q_translate[16 * 16 * 16];
/*
 * Table for translating a nucleic DB sequence into a protein sequence.
 * Initialized with the genetic code of the DB sequences.
 */
static char d_translate[16 * 16 * 16];

/**
 * Initialises a translation table for a genetic code.
 *
 * @see us_init_translation()
 * @see us_translate_sequence()
 *
 * @param tableno   the genetic code
 * @param table     the table, to initialise
 */
static void init_translate_table( int tableno, char * table ) {
    char remap[] = { 2, 1, 3, 0 };

    for( int a = 0; a < 16; a++ ) {
        for( int b = 0; b < 16; b++ ) {
            for( int c = 0; c < 16; c++ ) {
                char aa = '-';

                for( int i = 0; i < 4; i++ ) {
                    for( int j = 0; j < 4; j++ ) {
                        for( int k = 0; k < 4; k++ ) {
                            if( (a & (1 << i)) && (b & (1 << j)) && (c & (1 << k)) ) {
                                int codon = remap[i] * 16 + remap[j] * 4 + remap[k];

                                char x = code[tableno][codon];

                                if( aa == '-' ) {
                                    aa = x;
                                }
                                else if( aa == x ) {
                                    // do nothing
                                }
                                else if( (aa == 'B') && ((x == 'D') || (x == 'N')) ) {
                                    // do nothing
                                }
                                else if( (aa == 'D') && ((x == 'B') || (x == 'N')) ) {
                                    aa = 'B';
                                }
                                else if( (aa == 'N') && ((x == 'B') || (x == 'D')) ) {
                                    aa = 'B';
                                }
                                else if( (aa == 'Z') && ((x == 'Q') || (x == 'E')) ) {
                                    // do nothing
                                }
                                else if( (aa == 'E') && ((x == 'Z') || (x == 'Q')) ) {
                                    aa = 'Z';
                                }
                                else if( (aa == 'Q') && ((x == 'Z') || (x == 'E')) ) {
                                    aa = 'Z';
                                }
                                else {
                                    aa = 'X';
                                }
                            }
                        }
                    }
                }

                if( aa == '-' )
                    aa = 'X';

                table[256 * a + 16 * b + c] = map_ncbi_aa[(int) aa];
            }
        }
    }

#if 0
    /* dump it */

    outf("          -ACMGRSVTWYHKDBN\n");
    for(int x=0; x<16; x++)
    for(int y=0; y<16; y++) {
        outf("%2ld %2ld %c %c ", x, y, sym_ncbi_nt16[x], sym_ncbi_nt16[y]);
        for(int z=0; z<16; z++) {
            outf("%c", sym_ncbi_aa[table[256*x+16*y+z]]);
        }
        outf("\n");
    }
#endif
}

/**
 * Initialises the translation tables for query and DB sequences, using the
 * provided genetic codes.
 *
 * @see us_translate_sequence()
 *
 * @param qtableno  the genetic code of the query sequences
 * @param dtableno  the genetic code of the DB sequences
 */
void us_init_translation( int qtableno, int dtableno ) {
    /*
     * The codes start at number 1, therefore do we subtract 1 here.
     * http://www.ncbi.nlm.nih.gov/Taxonomy/Utils/wprintgc.cgi
     */
    init_translate_table( qtableno - 1, q_translate );
    init_translate_table( dtableno - 1, d_translate );
}

/**
 * Returns a newly created sequence, with the mapped values of the input
 * sequence.
 *
 * @param seq   the sequence, to be mapped
 * @param map   the mapping to use
 * @return      the new mapped sequence
 */
void us_map_sequence( sequence_t orig, sequence_t mapped, const char* map ) {
    /*
     * TODO here always allocate memory for the unknown symbols ...
     * maybe add some verbose flag, to omit the output and collection in some cases
     */
//    char * unknown_symbols = xmalloc( orig.len );
//    size_t unknown_count = 0;

    char m;
    for( size_t i = 0; i < orig.len; i++ ) {
        if( (m = map[(int) orig.seq[i]]) >= 0 ) {
            mapped.seq[i] = m;
        }
        else {
//            if( orig.seq[i] != '\n' && orig.seq[i] != ' ' && orig.seq[i] != '\t' ) {
//                unknown_symbols[unknown_count++] = orig.seq[i];
//            }
            mapped.seq[i] = 0;
        }
    }

//    if( unknown_count > 0 ) {
//        unknown_symbols[unknown_count] = 0;
//
//        outf( "Unknown symbols found and omitted: '%s'\n", unknown_symbols );
//    }
//    free( unknown_symbols );

    mapped.seq[mapped.len] = 0;
}

/**
 * Translates a DNA sequence into a protein sequence, according the strand and
 * frame information. If db_sequence is set, the translation table, initialized
 * with the genetic code, of the DB is used. Otherwise the translation table,
 * initialized with the genetic code, of the query sequence is used.
 *
 * @param db_sequence   1 for translating a DB sequence 0 otherwise
 * @param dna       the DNA sequence to translate
 * @param strand    the strand, that is translated
 * @param frame     the frame, that is read for translation
 * @param prot_seq  the resulting protein sequence
 */
void us_translate_sequence( int db_sequence, sequence_t dna, int strand, int frame, sequence_t * prot_seq ) {
    char* ttable = (db_sequence) ? d_translate : q_translate;

    size_t c;
    size_t pos, ppos = 0;

    prot_seq->len = (dna.len - frame) / 3;
    prot_seq->seq = xrealloc( prot_seq->seq, prot_seq->len + 1 );

    // forward strand
    if( strand == 0 ) {
        pos = frame;
        while( ppos < prot_seq->len ) {
            c = dna.seq[pos++];
            c <<= 4;
            c |= dna.seq[pos++];
            c <<= 4;
            c |= dna.seq[pos++];

            if( c > (16 * 16 * 16) ) {
                outf( "Wrong data in sequence. Codon position out of range: %ld, at position %d, for protein sequence position: %d\n",
                        c, pos, ppos );
            }

            prot_seq->seq[ppos++] = ttable[c];
        }
    }
    // complementary strand, done in reverse complement
    else {
        pos = dna.len - 1 - frame;

        while( ppos < prot_seq->len ) {
            c = ntcompl[(int) (dna.seq[pos--])];
            c <<= 4;
            c |= ntcompl[(int) (dna.seq[pos--])];
            c <<= 4;
            c |= ntcompl[(int) (dna.seq[pos--])];

            if( c > (16 * 16 * 16) ) {
                outf( "Wrong data in sequence. Codon position out of range: %ld, at position %d, for protein sequence position: %d\n",
                        c, pos, ppos );
            }

            prot_seq->seq[ppos++] = ttable[c];
        }
    }

    prot_seq->seq[ppos] = 0;
}

sequence_t us_prepare_sequence( char * seq, size_t len, int f, int s ) {
    sequence_t result;

    sequence_t db_seq;
    db_seq.seq = seq;
    db_seq.len = len;

    sequence_t conv_seq = { xmalloc( db_seq.len + 1 ), db_seq.len };


    if( symtype == NUCLEOTIDE ) {
        us_map_sequence( db_seq, conv_seq, map_ncbi_nt16 );

        if( s == 2 ) {
            result = (sequence_t ) { xmalloc( db_seq.len + 1 ), db_seq.len };
            us_revcompl( conv_seq, result );
        }
        else {
            result = conv_seq;
        }
    }
    else if( (symtype == TRANS_DB) || (symtype == TRANS_BOTH) ) {
        us_map_sequence( db_seq, conv_seq, map_ncbi_nt16 );

        us_translate_sequence( 1, conv_seq, s, f, &result );
    }
    else {
        us_map_sequence( db_seq, conv_seq, map_ncbi_aa );

        result = conv_seq;
    }

    return result;
}

/**
 * Computes the reverse complement of the input sequence. If the input sequence
 * is empty, the function returns 0.
 *
 * @param seq   the sequence to reverse complement
 * @param len   the length of the sequence
 * @return      the reverse complement or 0 in case of an empty sequence
 */
void us_revcompl( sequence_t orig, sequence_t rc ) {
    if( orig.len != rc.len ) {
        rc.seq = xrealloc( rc.seq, orig.len );
        rc.len = orig.len;
    }

    for( size_t i = 0; i < orig.len; i++ ) {
        rc.seq[i] = ntcompl[(int) (orig.seq[orig.len - 1 - i])];
    }
    rc.seq[orig.len] = 0;
}
