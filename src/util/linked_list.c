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

#include "linked_list.h"
#include "util.h"

p_node ll_init( void * data ) {
    p_node head = xmalloc( sizeof(struct node_t) );
    head->data = data;

    head->previous = 0;
    head->next = 0;

    return head;
}

void ll_wind_back( p_node * node ) {
    if( !node )
        return;

    while( (*node)->previous ) {
        *node = (*node)->previous;
    }
}

void ll_push( p_node * current, void * data ) {
    p_node new_node = ll_init( data );

    if( current ) {
        new_node->previous = *current;
        (*current)->next = new_node;
    }
    new_node->data = data;

    *current = new_node;
}

void * ll_pop( p_node * current ) {
    if( !current )
        return 0;

    p_node old_node = *current;
    void * result = 0;
    if( current ) {
        result = (*current)->data;

        *current = old_node->previous;
        if( *current ) {
            (*current)->next = 0;
        }

        old_node->next = 0;
        old_node->data = 0;

        free( old_node );
    }

    return result;
}

void ll_clear( p_node * node ) {
    if( !node )
        return;

    while( *node ) {
        /*
         * TODO does not free the data stored in the nodes
         */
        ll_pop( node );
    }
}

size_t ll_size( p_node node ) {
    size_t size = 0;

    p_node tmp = node;
    while( tmp ) {
        tmp = tmp->previous;

        size++;
    }

    return size;
}

void ll_print( p_node * node ) {
    p_node current = *node;

    if( !current )
        puts( "the stack is empty" );
    else {
        while( current ) {
            outf( "%p ", current->data );

            current = current->next;
        }
        putchar( '\n' );
    }
}
