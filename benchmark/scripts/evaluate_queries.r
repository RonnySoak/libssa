#install.packages( "ggplot2" )
library(ggplot2)
#library(plyr)
library(reshape2)

read_query_timing <- function( file_name, suffix="", nr_desc_elements ) {
    idx_data = nr_desc_elements + 1

    timing = read.csv( file=file_name, header=FALSE, sep="," )

    config_reduced = t(timing[1:(idx_data-1)])
    timing_reduced = t(timing[idx_data:length(timing[1,])])

    config_labels = apply( config_reduced, 2, paste, collapse="," )

    assign( paste( "timing", suffix, sep="" ), timing, envir = .GlobalEnv )

    assign(  "config_reduced", config_reduced, envir = .GlobalEnv )
    assign(  "config_labels", config_labels, envir = .GlobalEnv )
    assign(  "timing_reduced", timing_reduced, envir = .GlobalEnv )

    assign(  paste( "meantiming", suffix, sep="" ), apply( timing_reduced, 2, mean ), envir = .GlobalEnv )
}

read_query_timing( "results/02_04_2015_queries", nr_desc_elements = 4 );

total_runtime_func( timing, nr_desc_element = 4 )

set_indices()

# extract performance data
meantiming_sw <- meantiming[idx_SW]
meantiming_nw <- meantiming[idx_NW]

meantiming_sw_avx <- meantiming_sw[idx_avx2[-(73:144)]]
meantiming_nw_avx <- meantiming_nw[(idx_avx2[-(1:72)] - 144)]

meantiming_sw_sse <- meantiming_sw[idx_sse41[-(73:144)]]
meantiming_nw_sse <- meantiming_nw[(idx_sse41[-(1:72)] - 144)]

meantiming_sw_avx_8 <- meantiming_sw_avx[seq(1, 72, 2)]
meantiming_sw_avx_16 <- meantiming_sw_avx[seq(2, 72, 2)]

meantiming_nw_avx_8 <- meantiming_nw_avx[seq(1, 72, 2)]
meantiming_nw_avx_16 <- meantiming_nw_avx[seq(2, 72, 2)]

meantiming_sw_sse_8 <- meantiming_sw_sse[seq(1, 72, 2)]
meantiming_sw_sse_16 <- meantiming_sw_sse[seq(2, 72, 2)]

meantiming_nw_sse_8 <- meantiming_nw_sse[seq(1, 72, 2)]
meantiming_nw_sse_16 <- meantiming_nw_sse[seq(2, 72, 2)]

# print query performance data
print_perf_queries <- function( time_sw_avx, time_sw_sse, time_nw_avx, time_nw_sse, title, bit_width ) {
    query_length_data = read.csv( file="results/query_length", header=FALSE, sep="," )

    data_sw_avx_16 = cbind( query_length_data[,2], time_sw_avx )
    data_sw_sse_16 = cbind( query_length_data[,2], time_sw_sse )

    data_nw_avx_16 = cbind( query_length_data[,2], time_nw_avx )
    data_nw_sse_16 = cbind( query_length_data[,2], time_nw_sse )

    colors = c( "red", "green", "blue", "orange" )

    plot( data_sw_sse_16, main = title, ylab = "Time (seconds)", xlab = "Query length (residues)", xlim = c(20, max( query_length_data[,2]) ), ylim= c(0.2, 60), col = colors[1], type = "o", log = "xy", pch = 16 )

    lines( data_sw_avx_16, col = colors[2], type = "o", pch = 16 )

    lines( data_nw_sse_16, col = colors[3], type = "o", pch = 16 )
    lines( data_nw_avx_16, col = colors[4], type = "o", pch = 16 )

    row_names = unique( apply( config_reduced[-1,], 2, paste, collapse = "," ) )

    if( bit_width == 8 ) {row_names = row_names[c(T, F)]} else {row_names = row_names[c(F, T)]}

    row_names = gsub( pattern = ",", replacement = ", ", x = row_names)
    row_names = gsub( pattern = "_", replacement = " ", x = row_names)
    row_names = gsub( pattern = "SSE41", replacement = "SSE", x = row_names)
    row_names = gsub( pattern = "AVX2", replacement = "AVX", x = row_names)

    legend( "topleft", legend = row_names, fill=colors, bty = 0 )
}

# read query name and legth

# 8 bit searches
print_perf_queries( meantiming_sw_avx_8, meantiming_sw_sse_8, meantiming_nw_avx_8, meantiming_nw_sse_8, "Performance per query length", 8 )

# 16 bit searcher
print_perf_queries( meantiming_sw_avx_16, meantiming_sw_sse_16, meantiming_nw_avx_16, meantiming_nw_sse_16, "Performance per query length", 16 )






