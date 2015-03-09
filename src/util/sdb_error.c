/*
 * sdb_error.c
 *
 *  Created on: Mar 4, 2015
 *      Author: Jakob Frielingsdorf
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
