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

#include "sdb_error.h"

#include "util.h"

#define DESC_COUNT 8

static const char * error_code_desc[DESC_COUNT] = {
        "Unable to open input data file",
        "Unable to close input data file",
        "Could not read line",
        "Illegal header line",
        "Unable to open output file for writing",
        "Could not close output file",
        "Unable to allocate enough memory",
        "Unable to reallocate enough memory"
};

static p_sdb_error_list e_list = NULL;

void sdb_add_error( sdb_error_code_t err ) {
    if( !e_list ) {
        e_list = sdb_malloc( sizeof(struct sdb_error_list) );
        e_list->codes = sdb_malloc( sizeof(sdb_error_code_t) );
        e_list->count = 1;
    }
    else {
        e_list->count++;
        e_list->codes = sdb_realloc( e_list->codes, e_list->count * sizeof(sdb_error_code_t) );
    }

    e_list->codes[e_list->count - 1] = err;
}

size_t sdb_get_error_count() {
    if( e_list ) {
        return e_list->count;
    }
    return 0;
}

p_sdb_error_list sdb_get_error_list() {
    return e_list;
}

const char * sdb_get_error_desc( sdb_error_code_t code ) {
    if( code >= DESC_COUNT ) {
        return "Unknown error code";
    }
    return error_code_desc[ code ];
}
