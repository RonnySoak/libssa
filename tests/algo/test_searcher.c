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

static p_search_result setup_searcher_test( int bit_width, int search_type, char * query_string, char * db_file,
        int hit_count, int symtype, int strands ) {
    init_symbol_translation( symtype, strands, 3, 3 );
    mat_init_constant_scoring( 1, -1 );
    p_query query = query_read_from_string( "short query", query_string );

    s_init( search_type, bit_width, query, hit_count );

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

static void do_searcher_test_simple( int bit_width, int search_type, long score ) {
    p_search_result res = setup_searcher_test( bit_width, search_type, "AT", "short_db.fas", 1, NUCLEOTIDE,
    FORWARD_STRAND );

    ck_assert_int_eq( 1, res->chunk_count );
    ck_assert_int_eq( 1, res->seq_count );

    ck_assert_int_eq( 1, res->heap->alloc );
    ck_assert_int_eq( 1, res->heap->count );

    elem_t e = res->heap->array[0];
    ck_assert_int_eq( 0, e.db_id );
    ck_assert_int_eq( 0, e.query_id );

    ck_assert_int_eq( score, e.score );

    exit_searcher_test( res );
}

START_TEST (test_searcher_simple_sw_64)
    {
        do_searcher_test_simple( BIT_WIDTH_64, SMITH_WATERMAN, 2 );
    }END_TEST

START_TEST (test_searcher_simple_nw_64)
    {
        do_searcher_test_simple( BIT_WIDTH_64, NEEDLEMAN_WUNSCH, -2 );
    }END_TEST

START_TEST (test_searcher_simple_sw_16)
    {
        do_searcher_test_simple( BIT_WIDTH_16, SMITH_WATERMAN, 2 );
    }END_TEST

START_TEST (test_searcher_simple_nw_16)
    {
        do_searcher_test_simple( BIT_WIDTH_16, NEEDLEMAN_WUNSCH, -2 );
    }END_TEST

static p_search_result init_translate_test( int bit_width, int search_type, char * query, char * db_file, int hit_count,
        int symtype, int strands ) {
    p_search_result res = setup_searcher_test( bit_width, search_type, query, db_file, hit_count, symtype, strands );

    minheap_sort( res->heap );

    ck_assert_int_eq( hit_count, res->heap->count );

    return res;
}

static void do_multiple_sequence_test( int bit_width, int search_type, int result[16] ) {
    p_search_result res = init_translate_test( bit_width, search_type, "ATGCAAATTT", "short_nuc_db.fas", 8, NUCLEOTIDE,
    FORWARD_STRAND );

    p_minheap heap = res->heap;

    /*
     * Result contains tuple of the score and the database ID
     */
    for( int i = 0; i < 16; i += 2 ) {
        ck_assert_int_eq( result[i], heap->array[i / 2].score );
        ck_assert_int_eq( result[i + 1], heap->array[i / 2].db_id );
    }

    exit_searcher_test( res );
}

START_TEST (test_searcher_multiple_sequences_nw_64)
    {
        int result[16] = { -1, 0, -3, 3, -4, 7, -4, 6, -4, 4, -5, 2, -6, 5, -6, 1 };

        do_multiple_sequence_test( BIT_WIDTH_64, NEEDLEMAN_WUNSCH, result );
    }END_TEST

START_TEST (test_searcher_multiple_sequences_nw_16)
    {
        int result[16] = { -1, 0, -3, 3, -4, 7, -4, 6, -4, 4, -5, 2, -6, 5, -6, 1 };

        do_multiple_sequence_test( BIT_WIDTH_16, NEEDLEMAN_WUNSCH, result );
    }END_TEST

START_TEST (test_searcher_multiple_sequences_sw_64)
    {
        int result[16] = { 4, 3, 4, 1, 4, 0, 3, 7, 3, 5, 3, 4, 3, 2, 2, 6 };

        do_multiple_sequence_test( BIT_WIDTH_64, SMITH_WATERMAN, result );
    }END_TEST

START_TEST (test_searcher_multiple_sequences_sw_16)
    {
        int result[16] = { 4, 3, 4, 1, 4, 0, 3, 7, 3, 5, 3, 4, 3, 2, 2, 6 };

        do_multiple_sequence_test( BIT_WIDTH_64, SMITH_WATERMAN, result );
    }END_TEST

static void test_translate_result( p_search_result res, int * result, int result_len ) {
    minheap_sort( res->heap );

    ck_assert_int_eq( result_len / 2, res->heap->count );

    p_minheap heap = res->heap;

    /*
     * Result contains tuple of the score and the database ID
     */
    for( int i = 0; i < result_len; i += 2 ) { // TODO test query ID as well!!
        ck_assert_int_eq( result[i], heap->array[i / 2].score );
        ck_assert_int_eq( result[i + 1], heap->array[i / 2].db_id );
    }

    exit_searcher_test( res );
}

START_TEST (test_searcher_translate_nw_64)
    {
        int result[12] = { -507, 0, -508, 0, -509, 0, -509, 0, -510, 0, -511, 0 };

        p_search_result res = init_translate_test( BIT_WIDTH_64, NEEDLEMAN_WUNSCH,
                "ATGCCCAAAATTGACTTGAATAGGGGCGTGAGGAGGGGTTTTCAATACTATTTTTTGAGGACGATGTATAA", "NP_009305.1.fas", 6,
                TRANS_QUERY,
                BOTH_STRANDS );

        test_translate_result( res, result, 12 );
    }END_TEST

START_TEST (test_searcher_translate_nw_16)
    {
        int result[12] = { -507, 0, -508, 0, -509, 0, -509, 0, -510, 0, -511, 0 };

        p_search_result res = init_translate_test( BIT_WIDTH_16, NEEDLEMAN_WUNSCH,
                "ATGCCCAAAATTGACTTGAATAGGGGCGTGAGGAGGGGTTTTCAATACTATTTTTTGAGGACGATGTATAA", "NP_009305.1.fas", 6,
                TRANS_QUERY,
                BOTH_STRANDS );

        test_translate_result( res, result, 12 );
    }END_TEST

START_TEST (test_searcher_translate_sw_64)
    {
        int result[12] = { 4, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0 };

        p_search_result res = init_translate_test( BIT_WIDTH_64, SMITH_WATERMAN,
                "ATGCCCAAAATTGACTTGAATAGGGGCGTGAGGAGGGGTTTTCAATACTATTTTTTGAGGACGATGTATAA", "NP_009305.1.fas", 6,
                TRANS_QUERY,
                BOTH_STRANDS );

        test_translate_result( res, result, 12 );
    }END_TEST

START_TEST (test_searcher_translate_sw_16)
    {
        int result[12] = { 4, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0 };

        p_search_result res = init_translate_test( BIT_WIDTH_16, SMITH_WATERMAN,
                "ATGCCCAAAATTGACTTGAATAGGGGCGTGAGGAGGGGTTTTCAATACTATTTTTTGAGGACGATGTATAA", "NP_009305.1.fas", 6,
                TRANS_QUERY,
                BOTH_STRANDS );

        test_translate_result( res, result, 12 );
    }END_TEST

START_TEST (test_searcher_AA_nw_64)
    {
        int result[12] = { -376, 0 };

        p_search_result res = init_translate_test( BIT_WIDTH_64, NEEDLEMAN_WUNSCH,
                "HPEVYILIIPGFGIISHVVSTYSKKPVFGEISMVYAMASIGLLGFLVWSHHMYIVGLDADTRAYFTSATMIIAIPTGIKI", "NP_009305.1.fas",
                1,
                AMINOACID,
                FORWARD_STRAND );

        test_translate_result( res, result, 2 );
    }END_TEST

START_TEST (test_searcher_AA_nw_16)
    {
        int result[12] = { -376, 0 };

        p_search_result res = init_translate_test( BIT_WIDTH_16, NEEDLEMAN_WUNSCH,
                "HPEVYILIIPGFGIISHVVSTYSKKPVFGEISMVYAMASIGLLGFLVWSHHMYIVGLDADTRAYFTSATMIIAIPTGIKI", "NP_009305.1.fas",
                1,
                AMINOACID,
                FORWARD_STRAND );

        test_translate_result( res, result, 2 );
    }END_TEST

START_TEST (test_searcher_AA_sw_64)
    {
        int result[12] = { 80, 0 };

        p_search_result res = init_translate_test( BIT_WIDTH_64, SMITH_WATERMAN,
                "HPEVYILIIPGFGIISHVVSTYSKKPVFGEISMVYAMASIGLLGFLVWSHHMYIVGLDADTRAYFTSATMIIAIPTGIKI", "NP_009305.1.fas",
                1,
                AMINOACID,
                FORWARD_STRAND );

        test_translate_result( res, result, 2 );
    }END_TEST

START_TEST (test_searcher_AA_sw_16)
    {
        int result[12] = { 80, 0 };

        p_search_result res = init_translate_test( BIT_WIDTH_16, SMITH_WATERMAN,
                "HPEVYILIIPGFGIISHVVSTYSKKPVFGEISMVYAMASIGLLGFLVWSHHMYIVGLDADTRAYFTSATMIIAIPTGIKI", "NP_009305.1.fas",
                1,
                AMINOACID,
                FORWARD_STRAND );

        test_translate_result( res, result, 2 );
    }END_TEST

START_TEST (test_init_search_data)
    {
        init_symbol_translation( NUCLEOTIDE, FORWARD_STRAND, 3, 3 );
        p_query query = query_read_from_string( "short query",
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
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
        p_query query = query_read_from_string( "short query",
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
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
        p_query query = query_read_from_string( "short query",
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
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
        p_query query = query_read_from_string( "short query",
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
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
        p_query query = query_read_from_string( "short query",
                "ATGCCCAAGCTGAATAGCGTAGAGGGGTTTTCATCATTTGAGGACGATGTATAA" );
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
    tcase_add_test( tc_core, test_searcher_multiple_sequences_nw_64 );
    tcase_add_test( tc_core, test_searcher_multiple_sequences_nw_16 );
    tcase_add_test( tc_core, test_searcher_multiple_sequences_sw_64 );
    tcase_add_test( tc_core, test_searcher_multiple_sequences_sw_16 );
    tcase_add_test( tc_core, test_searcher_translate_nw_64 );
    tcase_add_test( tc_core, test_searcher_translate_nw_16 );
    tcase_add_test( tc_core, test_searcher_translate_sw_64 );
    tcase_add_test( tc_core, test_searcher_translate_sw_16 );
    tcase_add_test( tc_core, test_searcher_AA_nw_64 );
    tcase_add_test( tc_core, test_searcher_AA_nw_16 );
    tcase_add_test( tc_core, test_searcher_AA_sw_64 );
    tcase_add_test( tc_core, test_searcher_AA_sw_16 );
    tcase_add_test( tc_core, test_init_search_data );
    tcase_add_test( tc_core, test_init_search_data2 );
    tcase_add_test( tc_core, test_init_search_data3 );
    tcase_add_test( tc_core, test_init_search_data4 );
    tcase_add_test( tc_core, test_init_search_data5 );

    suite_add_tcase( s, tc_core );
}
