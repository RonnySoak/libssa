#install.packages( "ggplot2" )
#library(ggplot2)
#library(plyr)
#library(reshape2)

get_indices <- function( config_datum ) {
    which( config_reduced == config_datum, useNames = F,  arr.ind=T )[, 2]
}

read_reduced_timing <- function( file_name, suffix="", query, nr_desc_elements ) {
    idx_data = nr_desc_elements + 1

    timing = read_results_csv( file_name )

    configtiming = t(timing[2:(idx_data-1)])
    datatiming = t(timing[idx_data:length(timing[1,])])

    config_reduced = configtiming[, seq( 1, ncol(datatiming), by=4 )]
    config_labels = apply( config_reduced, 2, paste, collapse="," )

    #timing_reduced = sapply( seq( 1, ncol(datatiming), by=4 ), function(x) rowSums(datatiming[, x:(x+3)]) )
    #timing_reduced = apply( timing_reduced, c(1, 2), function(x) (x / 4))

    timing_reduced = datatiming[,which( timing == query, useNames = F,  arr.ind=T )[, 1]]

    assign( paste( "timing", suffix, sep="" ), timing, envir = .GlobalEnv )

    assign(  "config_reduced", config_reduced, envir = .GlobalEnv )
    assign(  "config_labels", config_labels, envir = .GlobalEnv )
    assign(  paste( "timing_reduced", suffix, sep="" ), timing_reduced, envir = .GlobalEnv )

    assign(  paste( "maxtiming", suffix, sep="" ), apply( timing_reduced, 2, max ), envir = .GlobalEnv )
    assign(  paste( "mintiming", suffix, sep="" ), apply( timing_reduced, 2, min ), envir = .GlobalEnv )
    assign(  paste( "meantiming", suffix, sep="" ), apply( timing_reduced, 2, mean ), envir = .GlobalEnv )
    assign(  paste( "mediantiming", suffix, sep="" ), apply( timing_reduced, 2, median ), envir = .GlobalEnv )
    assign(  paste( "totaltiming", suffix, sep="" ), apply( timing_reduced, 2, sum ), envir = .GlobalEnv )
}

set_indices <- function() {
    assign( "idx_avx2", get_indices( "AVX2" ), envir = .GlobalEnv )
    assign( "idx_sse41", get_indices( "SSE41" ), envir = .GlobalEnv )
    assign( "idx_nosimd", get_indices( "NO_SIMD" ), envir = .GlobalEnv )

    assign( "idx_SW", get_indices( "SW" ), envir = .GlobalEnv )
    assign( "idx_NW", get_indices( "NW" ), envir = .GlobalEnv )

    assign( "idx_8bit", get_indices( "8_bit" ), envir = .GlobalEnv )
    assign( "idx_16bit", get_indices( "16_bit" ), envir = .GlobalEnv )
    assign( "idx_64bit", get_indices( "64_bit" ), envir = .GlobalEnv )

    assign( "idx_1t", get_indices( "1_t" ), envir = .GlobalEnv )
    assign( "idx_4t", get_indices( "4_t" ), envir = .GlobalEnv )
    assign( "idx_8t", get_indices( "8_t" ), envir = .GlobalEnv )
}

get_x_labels <- function( idx_col_a, idx_col_b, config_reduced ) {
    config_idx = c( idx_col_a, idx_col_b )
    config_idx = config_idx[ which( !duplicated( config_idx ) ) ]

    config_reduced = config_reduced[ ,config_idx ]

    x_names = apply( config_reduced, 2, paste, collapse="," )
    x_names = x_names[ !duplicated( x_names ) ]

    x_names
}

total_runtime_func <- function( timing, nr_desc_element ) {
    runtime = sum( timing[,(nr_desc_element+1):(10 + nr_desc_element)] )

    hours = as.integer( runtime / 3600 )
    minutes = as.integer( runtime %% 3600 / 60 )

    cat( 'Total runtime: ', hours, ':', minutes, 'hours' )
}
