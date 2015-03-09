/*
 * sdb_error.h
 *
 *  Created on: Sep 24, 2014
 *      Author: Jakob Frielingsdorf
 */

#ifndef SDB_ERROR_H_
#define SDB_ERROR_H_

#include <stddef.h>

typedef enum {
    DB_NOT_FOUND = 0,
    DB_NOT_CLOSED = 1,
    DB_LINE_NOT_READ = 2,
    DB_ILLEGAL_HEADER = 3,
    OUT_FILE_NOT_FOUND = 4,
    OUT_FILE_NOT_CLOSED = 5,
    MALLOC_NOT_ENOUGH_MEM = 6,
    REALLOC_NOT_ENOUGH_MEM = 7
} sdb_error_code_t;

struct sdb_error_list {
    sdb_error_code_t * codes;
    size_t count;
};
typedef struct sdb_error_list * p_sdb_error_list;

void sdb_add_error( sdb_error_code_t err );

size_t sdb_get_error_count();

p_sdb_error_list sdb_get_error_list();

const char * sdb_get_error_desc( sdb_error_code_t err );

#endif /* SDB_ERROR_H_ */
