#install.packages( "ggplot2" )
#library(ggplot2)
#library(plyr)
#library(reshape2)

read_reduced_timing <- function( file_name, suffix="", nr_desc_elements = 5 ) {
    idx_data = nr_desc_elements + 1

    timing = read.csv( file=file_name, header=FALSE, sep="," )

    configtiming = t(timing[2:(idx_data-1)])
    datatiming = t(timing[idx_data:length(timing[1,])])

    config_reduced = configtiming[, seq( 1, ncol(datatiming), by=4 )]
    config_labels = apply( config_reduced, 2, paste, collapse="," )

    timing_reduced = sapply( seq( 1, ncol(datatiming), by=4 ), function(x) rowSums(datatiming[, x:(x+3)]) )
    timing_reduced = apply( timing_reduced, c(1, 2), function(x) (x / 4))

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
    assign( "idx_avx2", which( config_reduced == "AVX2", arr.ind=T )[, 2], envir = .GlobalEnv )
    assign( "idx_sse41", which( config_reduced == "SSE41", arr.ind=T )[,2], envir = .GlobalEnv )
    assign( "idx_nosimd", which( config_reduced == "NO_SIMD", arr.ind=T )[,2], envir = .GlobalEnv )

    assign( "idx_SW", which( config_reduced == "SW", arr.ind=T )[,2], envir = .GlobalEnv )
    assign( "idx_NW", which( config_reduced == "NW", arr.ind=T )[,2], envir = .GlobalEnv )

    assign( "idx_8bit", which( config_reduced == "8_b", arr.ind=T )[,2], envir = .GlobalEnv )
    assign( "idx_16bit", which( config_reduced == "16_b", arr.ind=T )[,2], envir = .GlobalEnv )
    assign( "idx_64bit", which( config_reduced == "64_b", arr.ind=T )[,2], envir = .GlobalEnv )

    assign( "idx_1t", which( config_reduced == "1_t", arr.ind=T )[,2], envir = .GlobalEnv )
    assign( "idx_4t", which( config_reduced == "4_t", arr.ind=T )[,2], envir = .GlobalEnv )
    assign( "idx_8t", which( config_reduced == "8_t", arr.ind=T )[,2], envir = .GlobalEnv )
}

get_x_labels <- function( idx_col_a, idx_col_b, config_reduced ) {
    config_idx = c( idx_col_a, idx_col_b )
    config_idx = config_idx[ which( !duplicated( config_idx ) ) ]

    config_reduced = config_reduced[ ,config_idx ]

    x_names = apply( config_reduced, 2, paste, collapse="," )
    x_names = x_names[ !duplicated( x_names ) ]

    x_names
}

# comparing configurations
compare_plot_func <- function( idx_dim1_a, idx_dim1_b, idx_dim2, vdata, config_reduced, legend_text, title ) {
    idx_1a_2 = c(idx_dim1_a, idx_dim2)
    idx_1b_2 = c(idx_dim1_b, idx_dim2)

    vdata_a = vdata[ idx_1a_2[ which( duplicated( idx_1a_2 ) ) ] ]
    vdata_b = vdata[ idx_1b_2[ which( duplicated( idx_1b_2 ) ) ] ]

    plot_data = matrix( c( vdata_a, vdata_b ), ncol=length(vdata_a), byrow=T )

    x_names = get_x_labels( idx_dim1_a, idx_dim1_b, config_reduced )
    x_names = gsub( pattern = ",", replacement = ", ", x = x_names)
    x_names = gsub( pattern = "NO_SIMD, ", replacement = "", x = x_names)
    x_names = gsub( pattern = "_", replacement = " ", x = x_names)

    x = barplot( plot_data, main=title, ylab="Time (seconds)", xlab="Configurations", col=c("orange","darkblue"), beside=T, ylim=c(0, 20), las=1, space=c(0.2,0.8) )

    text( cex = 0.8, x = colMeans(x)+0.8, y = -1.25, labels = x_names, xpd = TRUE, srt = 45, pos = 2 )

    legend( "topright", legend=legend_text, fill=c("orange","darkblue" ), bty = 0 )
}

# improvements
improvements_func <- function( idx_a, idx_b, vdata, config_reduced, title ) {
    data = matrix( c( vdata[ idx_a ], vdata[ idx_b ] ), ncol=length(idx_a), byrow=T )

    improvement = sapply( seq( 1, length(idx_a), 1), function(x) (data[1,x] / data[2,x]) )

    x_names = get_x_labels( idx_a, idx_b, config_reduced )

    improvement = data.frame( x_names, improvement )

    print( title )
    print( improvement )
}
