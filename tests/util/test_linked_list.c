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

#include "../tests.h"

#include "../../src/util/linked_list.h"

START_TEST (test_init)
    {
        int data = 42;

        p_node node = ll_init( &data );

        ck_assert_int_eq( data, *(int * )node->data );
        ck_assert_ptr_eq( 0, node->next );
        ck_assert_ptr_eq( 0, node->previous );

        free( node );
    }END_TEST

START_TEST (test_push)
    {
        int data = 42;

        p_node node = ll_init( &data );
        p_node head = node;

        int data2 = 13;

        ll_push( &node, &data2 );

        ck_assert_ptr_ne( node, head );
        ck_assert_ptr_eq( node->previous, head );
        ck_assert_ptr_eq( head->next, node );

        ck_assert_ptr_eq( 0, node->next );

        ck_assert_int_eq( data, *(int * )head->data );
        ck_assert_int_eq( data2, *(int * )node->data );

        free( node );
        free( head );

    }END_TEST

START_TEST (test_pop)
    {
        ck_assert_ptr_eq( 0, ll_pop( 0 ) );

        int data = 42;

        p_node node = ll_init( &data );
        p_node head = node;

        int data2 = 13;

        ll_push( &node, &data2 );

        int * pop_data = ll_pop( &node );
        ck_assert_int_eq( *pop_data, data2 );

        ck_assert_ptr_eq( node, head );
        ck_assert_ptr_eq( 0, node->next );

        pop_data = ll_pop( &node );
        ck_assert_int_eq( *pop_data, data );

        ck_assert_ptr_eq( 0, node );
    }END_TEST

START_TEST (test_clear)
    {
        ll_clear( 0 );

        int data = 42;

        p_node node = ll_init( &data );

        ll_push( &node, &data );
        ll_push( &node, &data );
        ll_push( &node, &data );

        ll_clear( &node );

        ck_assert_ptr_eq( 0, node );
    }END_TEST

START_TEST (test_wind_back)
    {
        ll_wind_back( 0 );

        int data = 42;
        int data2 = 13;

        p_node node = ll_init( &data );
        ll_push( &node, &data2 );
        ll_push( &node, &data2 );
        ll_push( &node, &data2 );
        ll_push( &node, &data2 );

        p_node tail = node;

        ck_assert_int_eq( data2, *(int * )node->data );

        ll_wind_back( &node );

        ck_assert_int_eq( data, *(int * )node->data );
        ck_assert_ptr_eq( 0, node->previous );

        ll_clear( &tail );
        ck_assert_ptr_eq( 0, tail );
    }END_TEST

START_TEST (test_size)
    {
        ck_assert_int_eq( 0, ll_size( 0 ) );

        int data = 42;
        int data2 = 13;

        p_node node = ll_init( &data );
        ll_push( &node, &data2 );
        ll_push( &node, &data2 );
        ll_push( &node, &data2 );
        ll_push( &node, &data2 );

        ck_assert_int_eq( 5, ll_size( node ) );

        ll_clear( &node );
    }END_TEST

void addLinkedListTC( Suite *s ) {
    TCase *tc_core = tcase_create( "linkedlist" );
    tcase_add_test( tc_core, test_init );
    tcase_add_test( tc_core, test_push );
    tcase_add_test( tc_core, test_pop );
    tcase_add_test( tc_core, test_clear );
    tcase_add_test( tc_core, test_wind_back );
    tcase_add_test( tc_core, test_size );
    suite_add_tcase( s, tc_core );
}

