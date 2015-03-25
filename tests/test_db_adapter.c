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

#include "../src/db_adapter.h"
#include "tests.h"

#include "../src/util/util.h"
#include "../src/libssa_extern_db.h"
#include "../src/query.h"
#include "../src/util/util_sequence.h"

char * get_mapped_sequence( sequence_t orig ) {
    sequence_t mapped = { xmalloc( orig.len + 1 ), orig.len };
    us_map_sequence( orig, mapped, sym_ncbi_nt16u );
    return mapped.seq;
}

/**
 * Simple reverse complement implementation for upper case nucleotide sequences.
 */
char* revcompl( char* seq, size_t len ) {
    char* rc = xmalloc( len + 1 );

    for( size_t i = 0; i < len; i++ ) {
        switch( (int) seq[len - 1 - i] ) {
        case 65:
            rc[i] = 'T';
            break;
        case 67:
            rc[i] = 'G';
            break;
        case 71:
            rc[i] = 'C';
            break;
        case 84:
        case 85:
            rc[i] = 'A';
            break;
        }
    }
    rc[len] = 0;

    return rc;
}

START_TEST (test_init)
    {
        // should return doing nothing
        adp_exit();

        symtype = NUCLEOTIDE;
        adp_init( 1 );
        adp_exit();

        symtype = TRANS_QUERY;
        adp_init( 1 );
        adp_exit();

        symtype = TRANS_DB;
        adp_init( 1 );
        adp_exit();

        symtype = TRANS_BOTH;
        adp_init( 1 );
        adp_exit();

        symtype = AMINOACID;
        adp_init( 1 );
        adp_exit();
    }END_TEST

START_TEST (test_next_empty)
    {
        symtype = TRANS_DB;

        adp_init( 1 );

        p_db_chunk chunk = adp_init_new_chunk();
        adp_next_chunk( chunk );
        ck_assert_int_eq( 0, chunk->fill_pointer );

        adp_exit();
    }END_TEST

START_TEST (test_next_one_nuc_forward)
    {
        ssa_db_init( "tests/testdata/one_seq.fas" );

        symtype = NUCLEOTIDE;
        query_strands = FORWARD_STRAND;

        adp_init( 1 );

        p_db_chunk chunk = adp_init_new_chunk();
        adp_next_chunk( chunk );
        ck_assert_int_eq( 1, chunk->fill_pointer );

        p_sdb_sequence seq = chunk->seq[0];
        ck_assert_int_eq( 0, seq->ID );
        ck_assert_int_eq( 54, seq->seq.len );
        ck_assert_str_eq( "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA", get_mapped_sequence( seq->seq ) );
        ck_assert_int_eq( 0, seq->strand );
        ck_assert_int_eq( 0, seq->frame );

        // check for the end of sequences
        adp_next_chunk( chunk );
        ck_assert_int_eq( 0, chunk->fill_pointer );

        adp_free_chunk( chunk );

        adp_exit();
        ssa_db_close();

    }END_TEST

START_TEST (test_next_one_nuc_both)
    {
        ssa_db_init( "tests/testdata/one_seq.fas" );

        symtype = NUCLEOTIDE;
        query_strands = BOTH_STRANDS;
        adp_init( 1 );

        p_db_chunk chunk = adp_init_new_chunk();
        adp_next_chunk( chunk );

        ck_assert_int_eq( 2, chunk->fill_pointer );

        p_sdb_sequence seq = chunk->seq[0];
        ck_assert_int_eq( 0, seq->ID );
        ck_assert_int_eq( 54, seq->seq.len );
        ck_assert_str_eq( "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA", get_mapped_sequence( seq->seq ) );
        ck_assert_int_eq( 0, seq->strand );
        ck_assert_int_eq( 0, seq->frame );

        // get reverse complement strand

        seq = chunk->seq[1];
        ck_assert_int_eq( 0, seq->ID );
        ck_assert_int_eq( 54, seq->seq.len );
        ck_assert_str_eq( revcompl( "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA", seq->seq.len ),
                get_mapped_sequence( seq->seq ) );
        ck_assert_int_eq( 1, seq->strand );
        ck_assert_int_eq( 0, seq->frame );

        // check for the end of sequences
        adp_next_chunk( chunk );
        ck_assert_int_eq( 0, chunk->fill_pointer );

        adp_free_chunk( chunk );

        adp_exit();
        ssa_db_close();
    }END_TEST

START_TEST (test_next_one_db_translate_forward)
    {
        // test with forward strand and db translation
        ssa_db_init( "tests/testdata/one_seq.fas" );

        symtype = TRANS_DB;
        query_strands = FORWARD_STRAND;

        us_init_translation( 1, 3 );

        adp_init( 3 );

        p_db_chunk chunk = adp_init_new_chunk();
        adp_next_chunk( chunk );

        ck_assert_int_eq( 3, chunk->fill_pointer );

        p_sdb_sequence seq = chunk->seq[0];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 18, seq->seq.len );
        ck_converted_prot_eq( "MPKTNSVEGFSSFEDDV*", seq->seq );
        ck_assert_int_eq( 1, seq->strand );
        ck_assert_int_eq( 0, seq->frame );

        // 2.
        seq = chunk->seq[1];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 17, seq->seq.len );
        ck_converted_prot_eq( "CPSWMA*RGFHHLRTMY", seq->seq );
        ck_assert_int_eq( 1, seq->strand );
        ck_assert_int_eq( 1, seq->frame );

        // 3.
        seq = chunk->seq[2];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 17, seq->seq.len );
        ck_converted_prot_eq( "AQAE*RRGVFIIWGRCM", seq->seq );
        ck_assert_int_eq( 1, seq->strand );
        ck_assert_int_eq( 2, seq->frame );

        // check for the end of sequences
        adp_next_chunk( chunk );
        ck_assert_int_eq( 0, chunk->fill_pointer );

        adp_free_chunk( chunk );

        adp_exit();
        ssa_db_close();
    }END_TEST

START_TEST (test_next_one_db_translate_both)
    {
        // test with both strand and db translation
        ssa_db_init( "tests/testdata/one_seq.fas" );

        symtype = TRANS_DB;
        query_strands = BOTH_STRANDS;

        us_init_translation( 1, 3 );

        adp_init( 6 );

        p_db_chunk chunk = adp_init_new_chunk();
        adp_next_chunk( chunk );

        ck_assert_int_eq( 6, chunk->fill_pointer );

        p_sdb_sequence seq = chunk->seq[0];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 18, seq->seq.len );
        ck_converted_prot_eq( "MPKTNSVEGFSSFEDDV*", seq->seq );
        ck_assert_int_eq( 0, seq->strand );
        ck_assert_int_eq( 0, seq->frame );

        // 2.
        seq = chunk->seq[1];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 17, seq->seq.len );
        ck_converted_prot_eq( "CPSWMA*RGFHHLRTMY", seq->seq );
        ck_assert_int_eq( 0, seq->strand );
        ck_assert_int_eq( 1, seq->frame );

        // 3.
        seq = chunk->seq[2];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 17, seq->seq.len );
        ck_converted_prot_eq( "AQAE*RRGVFIIWGRCM", seq->seq );
        ck_assert_int_eq( 0, seq->strand );
        ck_assert_int_eq( 2, seq->frame );

        // 4.
        seq = chunk->seq[3];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 18, seq->seq.len );
        ck_converted_prot_eq( "LYIVTKWWKPTYAIQTGH", seq->seq );
        ck_assert_int_eq( 1, seq->strand );
        ck_assert_int_eq( 0, seq->frame );

        // 5.
        seq = chunk->seq[4];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 17, seq->seq.len );
        ck_converted_prot_eq( "YTSSSNDENPSTTFSLG", seq->seq );
        ck_assert_int_eq( 1, seq->strand );
        ck_assert_int_eq( 1, seq->frame );

        // 6.
        seq = chunk->seq[5];
        ck_assert_ptr_eq( 0, seq->ID );
        ck_assert_int_eq( 17, seq->seq.len );
        ck_converted_prot_eq( "MHRPQMMKTPTRYSAWA", seq->seq );
        ck_assert_int_eq( 1, seq->strand );
        ck_assert_int_eq( 2, seq->frame );

        // check for the end of sequences
        adp_next_chunk( chunk );
        ck_assert_int_eq( 0, chunk->fill_pointer );

        adp_free_chunk( chunk );

        adp_exit();
        ssa_db_close();
    }END_TEST

START_TEST (test_next_chunk)
    {
        ssa_db_init( "tests/testdata/test.fas" );

        // test with only forward strand
        symtype = NUCLEOTIDE;
        query_strands = FORWARD_STRAND;

        int chunk_size = 3;

        adp_init( chunk_size );

        p_db_chunk chunk = adp_init_new_chunk();

        adp_next_chunk( chunk );

        // 1.
        ck_assert_int_eq( chunk_size, (int )chunk->fill_pointer );
        ck_assert_int_eq( 0, chunk->seq[0]->ID );
        ck_assert_int_eq( 1, chunk->seq[1]->ID );
        ck_assert_int_eq( 2, chunk->seq[2]->ID );

        // 2.
        adp_next_chunk( chunk );

        ck_assert_int_eq( 2, (int )chunk->fill_pointer );
        ck_assert_int_eq( 3, chunk->seq[0]->ID );
        ck_assert_int_eq( 4, chunk->seq[1]->ID );

        // check for the end of sequences
        adp_next_chunk( chunk );

        ck_assert_int_eq( 0, chunk->fill_pointer );

        adp_free_chunk( chunk );

        adp_exit();
        ssa_db_close();
    }END_TEST

void addDBAdapterTC( Suite *s ) {
    TCase *tc_core = tcase_create( "db_adapter" );
    tcase_add_test( tc_core, test_init );
    tcase_add_test( tc_core, test_next_empty );
    tcase_add_test( tc_core, test_next_one_db_translate_both );
    tcase_add_test( tc_core, test_next_one_db_translate_forward );
    tcase_add_test( tc_core, test_next_one_nuc_both );
    tcase_add_test( tc_core, test_next_one_nuc_forward );
    tcase_add_test( tc_core, test_next_chunk );

    suite_add_tcase( s, tc_core );
}

