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

#install.packages( "ggplot2" )
library(ggplot2)
#library(plyr)
library(reshape2)

source( "scripts/evaluate_config.r" )
source( "scripts/evaluate_functions.r" )

read_query_timing <- function( file_name, suffix="", nr_desc_elements ) {
    idx_data = nr_desc_elements + 1

    timing = read_results_csv( file_name )

    config_reduced = t(timing[1:(idx_data-1)])
    timing_reduced = t(timing[idx_data:length(timing[1,])])

    config_labels = apply( config_reduced, 2, paste, collapse="," )

    assign( paste( "timing", suffix, sep="" ), timing, envir = .GlobalEnv )

    assign(  "config_reduced", config_reduced, envir = .GlobalEnv )
    assign(  "config_labels", config_labels, envir = .GlobalEnv )
    assign(  "timing_reduced", timing_reduced, envir = .GlobalEnv )

    assign(  paste( "meantiming", suffix, sep="" ), apply( timing_reduced, 2, mean ), envir = .GlobalEnv )
}

read_query_timing( "02_04_2015_queries", nr_desc_elements = 4 );

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
plot_perf_queries <- function( time_avx_8, time_sse_8, time_avx_16, time_sse_16, title, algo ) {
    query_length_data = read_results_csv( "query_length" )

    data_avx_8 = cbind( query_length_data[,2], time_avx_8 )
    data_sse_8 = cbind( query_length_data[,2], time_sse_8 )

    data_avx_16 = cbind( query_length_data[,2], time_avx_16 )
    data_sse_16 = cbind( query_length_data[,2], time_sse_16 )

    colors = c( "red", "green3", "blue", "orange" )

    plot( data_sse_8, main = title, ylab = "Time (seconds)", xlab = "Query length (residues)", xlim = c(20, max( query_length_data[,2]) ), ylim= c(0.2, 60), col = colors[1], type = "o", log = "xy", pch = 16 )

    lines( data_sse_16, col = colors[2], type = "o", pch = 16 )

    lines( data_avx_8, col = colors[3], type = "o", pch = 16 )
    lines( data_avx_16, col = colors[4], type = "o", pch = 16 )

    row_names = unique( apply( config_reduced[-1,], 2, paste, collapse = "," ) )

    if( algo == "SW" ) {row_names = row_names[1:4]} else {row_names = row_names[5:8]}

    row_names = gsub( pattern = ",", replacement = ", ", x = row_names)
    row_names = gsub( pattern = "_", replacement = " ", x = row_names)
    row_names = gsub( pattern = "8 b", replacement = "  8 b", x = row_names)
    row_names = gsub( pattern = "SSE41", replacement = "SSE", x = row_names)
    row_names = gsub( pattern = "AVX2", replacement = "AVX", x = row_names)

    legend( "topleft", legend = row_names, fill=colors, bty = 0, bg = "white" )
}

# read query name and legth

# 8 bit searches
pdf(file = add_output_dir( "performance_per_querylength_SW" ), width = 9, height = 5, pointsize = 12)
plot_perf_queries( meantiming_sw_avx_8, meantiming_sw_sse_8, meantiming_sw_avx_16, meantiming_sw_sse_16, "Performance per query length", "SW" )
dev.off()

# 16 bit searcher
pdf(file= add_output_dir( "performance_per_querylength_NW" ), width = 9, height = 5, pointsize = 12)
plot_perf_queries( meantiming_nw_avx_8, meantiming_nw_sse_8, meantiming_nw_avx_16, meantiming_nw_sse_16, "Performance per query length", "NW" )
dev.off()

# deviation of the AVX over SSE improvement
plot_avx_improvement <- function( time_sw_8, time_sw_16, time_nw_8, time_nw_16, title, y_lim, conf_red ) {
    query_length_data = read_results_csv( "query_length" )

    data_sw_8 = cbind( query_length_data[,2], time_sw_8 )
    data_sw_16 = cbind( query_length_data[,2], time_sw_16 )

    data_nw_8 = cbind( query_length_data[,2], time_nw_8 )
    data_nw_16 = cbind( query_length_data[,2], time_nw_16 )

    colors = c( "red", "green3", "blue", "orange" )

    plot( data_sw_8, main = title, ylab = "Improvement (ratio)", xlab = "Query length (residues)", xlim = c(20, max( query_length_data[,2]) ), ylim= y_lim, col = colors[1], type = "o", log = "xy", pch = 16 )

    lines( data_sw_16, col = colors[2], type = "o", pch = 16 )

    lines( data_nw_8, col = colors[3], type = "o", pch = 16 )
    lines( data_nw_16, col = colors[4], type = "o", pch = 16 )

    row_names = unique( apply( config_reduced[-conf_red,], 2, paste, collapse = "," ) )

    row_names = gsub( pattern = ",", replacement = ", ", x = row_names)
    row_names = gsub( pattern = "_", replacement = " ", x = row_names)
    row_names = gsub( pattern = "8 b", replacement = "  8 b", x = row_names)
    row_names = gsub( pattern = "SSE41", replacement = "SSE", x = row_names)
    row_names = gsub( pattern = "AVX2", replacement = "AVX", x = row_names)

    legend( "topleft", legend = row_names, fill=colors, bty = 0, bg = "white" )
}


pdf(file= add_output_dir( "improvement_ratio_avx_sse" ), width = 9, height = 5, pointsize = 12)
plot_avx_improvement(meantiming_sw_sse_8 / meantiming_sw_avx_8, meantiming_sw_sse_16 / meantiming_sw_avx_16, meantiming_nw_sse_8 / meantiming_nw_avx_8, meantiming_nw_sse_16 / meantiming_nw_avx_16, "Improvement ratio of AVX over SSE per query length", c(1, 2), c(1:2) )
dev.off()


pdf(file= add_output_dir( "improvement_ratio_8_16_bit" ), width = 9, height = 5, pointsize = 12)
plot_avx_improvement(meantiming_sw_sse_16 / meantiming_sw_sse_8, meantiming_sw_avx_16 / meantiming_sw_avx_8, meantiming_nw_sse_16 / meantiming_nw_sse_8, meantiming_nw_avx_16 / meantiming_nw_avx_8, "Improvement ratio of 8 bit over 16 bit per query length", c(0.8, 2), c(1,4) )
dev.off()

