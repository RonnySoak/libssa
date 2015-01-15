/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/algo/aligner.h"
#include "../../src/algo/align.h"
#include "../../src/algo/manager.h"

#include "../../src/util/util.h"
#include "../../src/libssa.h"
#include "../../src/util/minheap.h"
#include "../../src/matrices.h"
#include "../../src/db_iterator.h"
#include "../../src/query.h"

extern p_alignment_data adp;

elem_t new_elem0( int ID, int frame, int strand, int qid, long score ) {
    elem_t e;
    e.db_id = ID;
    e.dframe = frame;
    e.dstrand = strand;
    e.query_id = qid;
    e.score = score;

    return e;
}

START_TEST (test_aligner_simple_sw)
    {
        mat_init_constant_scoring( 1, -1 );
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

        p_query query = query_read_from_string( "short query", "AT" );
        ssa_db_init_fasta( "./tests/testdata/short_db.fas" );

        it_init( 1 );


        sequence sdb = it_translate_sequence( ssa_db_get_sequence( 0 ), 0, 0 );

        elem_t e1 = new_elem0( 0, 0, 0, 0, 2 );
        elem_t * elements = { &e1 };

        init_for_sw( query, 1 );
        adp->pair_count = 1;
        adp->result_sequence_pairs = elements;
        set_alignment_data( adp );

        p_alignment_list alist = a_align( NULL );

        ck_assert_int_eq( 1, alist->len );

        alignment_p al = alist->alignments[0];

        ck_assert_str_eq( sdb.seq, al->db_seq.seq );
        ck_assert_int_eq( sdb.len, al->db_seq.len );
        ck_assert_int_eq( 0, al->db_seq.frame );
        ck_assert_int_eq( 0, al->db_seq.strand );
        ck_assert_int_eq( 0, al->db_seq.ID );

        ck_assert_str_eq( query->nt[0].seq, al->query.seq );
        ck_assert_int_eq( query->nt[0].len, al->query.len );
        ck_assert_int_eq( 0, al->query.frame );
        ck_assert_int_eq( 0, al->query.strand );

        ck_assert_int_eq( 2, al->score ); // the score is 2, since we set it to 2
        ck_assert_int_eq( 1, al->align_d_start );
        ck_assert_int_eq( 2, al->align_d_end );
        ck_assert_int_eq( 0, al->align_q_start );
        ck_assert_int_eq( 1, al->align_q_end );

        ck_assert_str_eq( "2M", al->alignment );

        a_free( alist );

        mat_free();
        query_free( query );
        it_free();
        ssa_db_free();
    }END_TEST

START_TEST (test_aligner_more_sequences_sw)
    {
        mat_init_constant_scoring( 1, -1 );
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

        p_query query = query_read_from_string( "query", "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
        ssa_db_init_fasta( "./tests/testdata/test.fas" );

        it_init( 3 );

        p_minheap heap = minheap_init( 5 );

        sequence sdb0 = it_translate_sequence( ssa_db_get_sequence( 0 ), 0, 0 );
        elem_t e = new_elem0( 0, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        e = new_elem0( 1, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        e = new_elem0( 2, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        e = new_elem0( 3, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        e = new_elem0( 4, 0, 0, 0, 2 );
        minheap_add( heap, &e );

        init_for_sw( query, 1 );
        adp->pair_count = 5;
        adp->result_sequence_pairs = heap->array;
        set_alignment_data( adp );

        p_alignment_list alist = a_align( NULL );

        ck_assert_int_eq( 5, alist->len );

        alignment_p al = alist->alignments[0];

        ck_assert_str_eq( sdb0.seq, al->db_seq.seq );
        ck_assert_int_eq( sdb0.len, al->db_seq.len );
        ck_assert_int_eq( 0, al->db_seq.frame );
        ck_assert_int_eq( 0, al->db_seq.strand );
        ck_assert_int_eq( 0, al->db_seq.ID );

        ck_assert_str_eq( query->nt[0].seq, al->query.seq );
        ck_assert_int_eq( query->nt[0].len, al->query.len );
        ck_assert_int_eq( 0, al->query.frame );
        ck_assert_int_eq( 0, al->query.strand );

        ck_assert_int_eq( 2, al->score ); // the score is 2, since we set it to 2
        ck_assert_int_eq( 1, al->align_d_start );
        ck_assert_int_eq( 120, al->align_d_end );
        ck_assert_int_eq( 1, al->align_q_start );
        ck_assert_int_eq( 53, al->align_q_end );
        ck_assert_str_eq( "DMI2MI2MIM3IM2IM4IM9I2MIM4I2M4I2M7IM6IMI3M"
                "3I2MI2M6ID3MIM3IM3IMIMD4M4IMI4MDMIMIMI3MDMI", al->alignment ); // TODO check this

        a_free( alist );
        minheap_exit( heap );

        it_free();
        mat_free();
        query_free( query );
        ssa_db_free();
    }END_TEST

START_TEST (test_aligner_simple_nw)
    {
        mat_init_constant_scoring( 1, -1 );
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );

        p_query query = query_read_from_string( "short query", "AT" );
        ssa_db_init_fasta( "./tests/testdata/short_db.fas" );

        it_init( 1 );

        sequence sdb = it_translate_sequence( ssa_db_get_sequence( 0 ), 0, 0 );

        elem_t e1 = new_elem0( 0, 0, 0, 0, 2 );
        elem_t * elements = { &e1 };

        init_for_nw( query, 1 );
        adp->pair_count = 1;
        adp->result_sequence_pairs = elements;
        set_alignment_data( adp );

        p_alignment_list alist = a_align( NULL );

        ck_assert_int_eq( 1, alist->len );

        alignment_p al = alist->alignments[0];

        ck_assert_str_eq( sdb.seq, al->db_seq.seq );
        ck_assert_int_eq( sdb.len, al->db_seq.len );
        ck_assert_int_eq( 0, al->db_seq.frame );
        ck_assert_int_eq( 0, al->db_seq.strand );
        ck_assert_int_eq( 0, al->db_seq.ID );

        ck_assert_str_eq( query->nt[0].seq, al->query.seq );
        ck_assert_int_eq( query->nt[0].len, al->query.len );
        ck_assert_int_eq( 0, al->query.frame );
        ck_assert_int_eq( 0, al->query.strand );

        ck_assert_int_eq( 2, al->score ); // the score is 2, since we set it to 2
        ck_assert_int_eq( 0, al->align_d_start );
        ck_assert_int_eq( 3, al->align_d_end );
        ck_assert_int_eq( 0, al->align_q_start );
        ck_assert_int_eq( 1, al->align_q_end );

        ck_assert_str_eq( "I2MI", al->alignment );

        a_free( alist );

        mat_free();
        query_free( query );
        it_free();
        ssa_db_free();
    }END_TEST

void addAlignerTC( Suite *s ) {
    TCase *tc_core = tcase_create( "aligner" );
    tcase_add_test( tc_core, test_aligner_simple_sw );
    tcase_add_test( tc_core, test_aligner_simple_nw );
    tcase_add_test( tc_core, test_aligner_more_sequences_sw );

    suite_add_tcase( s, tc_core );
}
