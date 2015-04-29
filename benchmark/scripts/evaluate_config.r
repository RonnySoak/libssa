#
#    Copyright (C) 2014-2015 Jakob Frielingsdorf
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Contact: Jakob Frielingsdorf <jfrielingsdorf@gmail.com>

output_dir <- "~/projects/master_thesis/tex/img"
result_dir <- "~/projects/libssa/benchmark/results"

# Set to 1, to 'tmp' is added to each filename. Set to 0, to omit the prefix 'tmp'
create_temp_files <- 1

add_output_dir <- function( filename ) {
    res = paste( output_dir, filename, sep = "/" )

    if( create_temp_files ) {res = paste( res, "tmp", sep = "_" )}

    paste( res, "pdf", sep = "." )
}

read_results_csv <- function( filename ) {
    res = paste( result_dir, filename, sep = "/" )

    read.csv( file=res, header=FALSE, sep="," )
}
