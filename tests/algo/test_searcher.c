/*
 * test_search63.c
 *
 *  Created on: 18 Jul 2014
 *      Author: kaos
 */

#include "../tests.h"

#include "../../src/util/util.h"
#include "../../src/libssa.h"
#include "../../src/util/minheap.h"
#include "../../src/matrices.h"
#include "../../src/db_iterator.h"
#include "../../src/query.h"
#include "../../src/algo/searcher.h"
#include "../../src/algo/search.h"

static p_query setup_searcher_test_init( int bit_width, int search_type, char * query_string, int hit_count,
        int symtype, int strands ) {
    init_symbol_translation( symtype, strands, 3, 3 );
    mat_init_constant_scoring( 1, -1 );

    p_query query = query_read_from_string( "short query", query_string );

    s_init( search_type, bit_width, query, hit_count );

    return query;
}

static p_search_result setup_searcher_test( int bit_width, int search_type, char * query_string, char * db_file,
        int hit_count ) {
    p_query query = setup_searcher_test_init( bit_width, search_type, query_string, hit_count, NUCLEOTIDE,
    FORWARD_STRAND );

    ssa_db_init_fasta( concat( "./tests/testdata/", db_file ) );

    gapO = 1;
    gapE = 1;

    it_init( hit_count );

    p_search_result res = s_search( NULL );

    minheap_sort( res->heap );

    query_free( query );

    return res;
}

static void exit_searcher_test( p_search_result res ) {
    s_free( res );
    it_free();
    mat_free();
}

START_TEST (test_searcher_simple_sw_64)
    {
        p_search_result res = setup_searcher_test( BIT_WIDTH_64, SMITH_WATERMAN, "AT", "short_db.fas", 1 );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 1, res->seq_count );

        ck_assert_int_eq( 1, res->heap->alloc );
        ck_assert_int_eq( 1, res->heap->count );

        elem_t e = res->heap->array[0];
        ck_assert_int_eq( 0, e.db_id );
        ck_assert_int_eq( 0, e.query_id );

        ck_assert_int_eq( 2, e.score );

        exit_searcher_test( res );
    }END_TEST

START_TEST (test_searcher_simple_nw_64)
    {
        p_search_result res = setup_searcher_test( BIT_WIDTH_64, NEEDLEMAN_WUNSCH, "AT", "short_db.fas", 1 );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 1, res->seq_count );

        ck_assert_int_eq( 1, res->heap->alloc );
        ck_assert_int_eq( 1, res->heap->count );

        elem_t e = res->heap->array[0];
        ck_assert_int_eq( 0, e.db_id );
        ck_assert_int_eq( 0, e.query_id );

        ck_assert_int_eq( -2, e.score );

        exit_searcher_test( res );
    }END_TEST

START_TEST (test_searcher_simple_sw_16)
    {
        p_search_result res = setup_searcher_test( BIT_WIDTH_16, SMITH_WATERMAN, "AT", "short_db.fas", 1 );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 1, res->seq_count );

        ck_assert_int_eq( 1, res->heap->alloc );
        ck_assert_int_eq( 1, res->heap->count );

        elem_t e = res->heap->array[0];
        ck_assert_int_eq( 0, e.db_id );
        ck_assert_int_eq( 0, e.query_id );

        ck_assert_int_eq( 2, e.score );

        exit_searcher_test( res );
    }END_TEST

START_TEST (test_searcher_simple_nw_16)
    {
        p_search_result res = setup_searcher_test( BIT_WIDTH_16, NEEDLEMAN_WUNSCH, "AT", "short_db.fas", 1 );

        ck_assert_int_eq( 1, res->chunk_count );
        ck_assert_int_eq( 1, res->seq_count );

        ck_assert_int_eq( 1, res->heap->alloc );
        ck_assert_int_eq( 1, res->heap->count );

        elem_t e = res->heap->array[0];
        ck_assert_int_eq( 0, e.db_id );
        ck_assert_int_eq( 0, e.query_id );

        ck_assert_int_eq( -2, e.score );

        exit_searcher_test( res );
    }END_TEST

START_TEST (test_init_search_data)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        p_query query = query_read_from_string( "short query", "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
        p_search_data sdp = s_create_searchdata( query, 1 );

        // query data
        ck_assert_int_eq( 1, sdp->q_count );

        seq_buffer buf = sdp->queries[0];

        ck_assert_str_eq( &query->nt[0].seq, &buf.seq.seq );
        ck_assert_int_eq( query->nt[0].len, buf.seq.len );
        ck_assert_int_eq( 0, buf.frame );
        ck_assert_int_eq( 0, buf.strand );

        ck_assert_int_eq( 0, sdp->queries[1].seq.len );
        ck_assert_int_eq( 0, sdp->queries[2].seq.len );
        ck_assert_int_eq( 0, sdp->queries[3].seq.len );
        ck_assert_int_eq( 0, sdp->queries[4].seq.len );
        ck_assert_int_eq( 0, sdp->queries[5].seq.len );

        ck_assert_int_eq( 1, sdp->hit_count );

        ck_assert_int_eq( 54, sdp->hearraylen );
        // TODO test profile

        query_free( query );
    }END_TEST

START_TEST (test_init_search_data2)
    {
        init_symbol_translation( NUCLEOTIDE, COMPLEMENTARY_STRAND, 3, 3 );
        p_query query = query_read_from_string( "short query", "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
        p_search_data sdp = s_create_searchdata( query, 5 );

        // query data
        ck_assert_int_eq( 1, sdp->q_count );

        seq_buffer buf = sdp->queries[0];
        ck_assert_str_eq( &query->nt[1].seq, &buf.seq.seq );
        ck_assert_int_eq( query->nt[1].len, buf.seq.len );
        ck_assert_int_eq( 0, buf.frame );
        ck_assert_int_eq( 1, buf.strand );

        ck_assert_int_eq( 0, sdp->queries[1].seq.len );
        ck_assert_int_eq( 0, sdp->queries[2].seq.len );
        ck_assert_int_eq( 0, sdp->queries[3].seq.len );
        ck_assert_int_eq( 0, sdp->queries[4].seq.len );
        ck_assert_int_eq( 0, sdp->queries[5].seq.len );

        ck_assert_int_eq( 5, sdp->hit_count );

        ck_assert_int_eq( 54, sdp->hearraylen );

        query_free( query );
    }END_TEST

START_TEST (test_init_search_data3)
    {
        init_symbol_translation( TRANS_QUERY, FORWARD_STRAND, 3, 3 );
        p_query query = query_read_from_string( "short query", "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
        p_search_data sdp = s_create_searchdata( query, 5 );

        // query data
        ck_assert_int_eq( 3, sdp->q_count );

        seq_buffer buf = sdp->queries[0];
        ck_assert_str_eq( &query->aa[0].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[0].len, buf.seq.len );
        ck_assert_int_eq( 0, buf.frame );
        ck_assert_int_eq( 0, buf.strand );

        buf = sdp->queries[1];
        ck_assert_str_eq( &query->aa[1].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[1].len, buf.seq.len );
        ck_assert_int_eq( 1, buf.frame );
        ck_assert_int_eq( 0, buf.strand );

        buf = sdp->queries[2];
        ck_assert_str_eq( &query->aa[2].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[2].len, buf.seq.len );
        ck_assert_int_eq( 2, buf.frame );
        ck_assert_int_eq( 0, buf.strand );

        ck_assert_int_eq( 0, sdp->queries[3].seq.len );
        ck_assert_int_eq( 0, sdp->queries[4].seq.len );
        ck_assert_int_eq( 0, sdp->queries[5].seq.len );

        ck_assert_int_eq( 5, sdp->hit_count );

        ck_assert_int_eq( 18, sdp->hearraylen );

        query_free( query );
    }END_TEST

START_TEST (test_init_search_data4)
    {
        init_symbol_translation( TRANS_QUERY, BOTH_STRANDS, 3, 3 );
        p_query query = query_read_from_string( "short query", "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
        p_search_data sdp = s_create_searchdata( query, 3 );

        // query data
        ck_assert_int_eq( 6, sdp->q_count );

        seq_buffer buf = sdp->queries[0];
        ck_assert_str_eq( &query->aa[0].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[0].len, buf.seq.len );
        ck_assert_int_eq( 0, buf.frame );
        ck_assert_int_eq( 0, buf.strand );

        buf = sdp->queries[1];
        ck_assert_str_eq( &query->aa[1].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[1].len, buf.seq.len );
        ck_assert_int_eq( 1, buf.frame );
        ck_assert_int_eq( 0, buf.strand );

        buf = sdp->queries[2];
        ck_assert_str_eq( &query->aa[2].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[2].len, buf.seq.len );
        ck_assert_int_eq( 2, buf.frame );
        ck_assert_int_eq( 0, buf.strand );

        buf = sdp->queries[3];
        ck_assert_str_eq( &query->aa[3].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[3].len, buf.seq.len );
        ck_assert_int_eq( 0, buf.frame );
        ck_assert_int_eq( 1, buf.strand );

        buf = sdp->queries[4];
        ck_assert_str_eq( &query->aa[4].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[4].len, buf.seq.len );
        ck_assert_int_eq( 1, buf.frame );
        ck_assert_int_eq( 1, buf.strand );

        buf = sdp->queries[5];
        ck_assert_str_eq( &query->aa[5].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[5].len, buf.seq.len );
        ck_assert_int_eq( 2, buf.frame );
        ck_assert_int_eq( 1, buf.strand );

        ck_assert_int_eq( 3, sdp->hit_count );

        ck_assert_int_eq( 18, sdp->hearraylen );

        query_free( query );
    }END_TEST

START_TEST (test_init_search_data5)
    {
        init_symbol_translation( AMINOACID, BOTH_STRANDS, 3, 3 );
        p_query query = query_read_from_string( "short query", "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
        p_search_data sdp = s_create_searchdata( query, 1 );

        // query data
        ck_assert_int_eq( 1, sdp->q_count );

        seq_buffer buf = sdp->queries[0];
        ck_assert_str_eq( &query->aa[0].seq, &buf.seq.seq );
        ck_assert_int_eq( query->aa[0].len, buf.seq.len );
        ck_assert_int_eq( 0, buf.frame );
        ck_assert_int_eq( 0, buf.strand );

        ck_assert_int_eq( 0, sdp->queries[1].seq.len );
        ck_assert_int_eq( 0, sdp->queries[2].seq.len );
        ck_assert_int_eq( 0, sdp->queries[3].seq.len );
        ck_assert_int_eq( 0, sdp->queries[4].seq.len );
        ck_assert_int_eq( 0, sdp->queries[5].seq.len );

        ck_assert_int_eq( 1, sdp->hit_count );

        ck_assert_int_eq( 54, sdp->hearraylen );

        query_free( query );
    }END_TEST

void addSearcherTC( Suite *s ) {
    TCase *tc_core = tcase_create( "searcher" );
    tcase_add_test( tc_core, test_searcher_simple_sw_64 );
    tcase_add_test( tc_core, test_searcher_simple_nw_64 );
    tcase_add_test( tc_core, test_searcher_simple_sw_16 );
    tcase_add_test( tc_core, test_searcher_simple_nw_16 );
    tcase_add_test( tc_core, test_init_search_data );
    tcase_add_test( tc_core, test_init_search_data2 );
    tcase_add_test( tc_core, test_init_search_data3 );
    tcase_add_test( tc_core, test_init_search_data4 );
    tcase_add_test( tc_core, test_init_search_data5 );

    suite_add_tcase( s, tc_core );
}
