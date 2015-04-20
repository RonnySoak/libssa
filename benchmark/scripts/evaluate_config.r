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
