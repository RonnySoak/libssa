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
