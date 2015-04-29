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

#read_reduced_timing( "14_02_2015_whole_lib" );
#read_reduced_timing( "18_02_2015_whole_lib" );
#read_reduced_timing( "19_02_2015_alignment_only", nr_desc_elements = 5 );
read_reduced_timing( "30_03_2015_alignment_only", "", "P19930", nr_desc_elements = 5 );
O74807 P19930 Q3ZAI3 P18080
total_runtime_func( timing, 5 )

set_indices()

# --------------------------------------------------------
# compare 8 bit to 16 bit, depending on the query sequence
col_names <- c( "O74807", "P19930", "Q3ZAI3", "P18080" )
query_length <- t(matrix( c( "O74807",110,"P19930",195,"Q3ZAI3",390,"P18080",513 ), nrow=2, ncol=4 ))

row_names <- unique( apply( config_reduced[-4,-c(idx_64bit, idx_8t, idx_4t)], 2, paste, collapse = "," ) )
row_names <- gsub( pattern = ",", replacement = ", ", x = row_names)
row_names <- gsub( pattern = "_", replacement = " ", x = row_names)
row_names <- gsub( pattern = "8 b", replacement = "  8 b", x = row_names)
row_names <- gsub( pattern = "SSE41", replacement = "SSE", x = row_names)
row_names <- gsub( pattern = "AVX2", replacement = "AVX", x = row_names)

timing_per_query <- timing[-which( timing == "64_bit", arr.ind=T )[,1],6:15]
avg_per_query <- apply( timing_per_query, 1, mean )

data_t = cbind( as.integer( query_length[,2] ), matrix( avg_per_query, nrow = 4, ncol = 8, byrow = F ))

n = length( row_names )
# colors = rainbow(n, s = 1, v = 1, start = 0, end = max(1, n - 1)/n, alpha = 1)
colors = c( "red", "orange", "green3", "blue" )
legend_colors = c( "red", "red", "orange", "orange", "blue", "blue", "green3", "green3" )

x_labels <- paste( query_length[,2], query_length[,1], sep = "\n" )

pdf(file = add_output_dir( "runtime_per_query_8_16_bit" ), width = 10, height = 5.5, pointsize = 12)
par( mar=c(5, 5, 3, 2) )
plot( data_t[,c(1,2)], main = "Runtimes per configuration and query sequence", xlab = "Query length (residues)", ylab = "Time (seconds)",  xlim=c(100,520), ylim= c(1.7, 25), col = colors[1], type = "o", pch = 16, xaxt = "n", log="xy" )
axis( 1, at = query_length[,2] )

lines( data_t[,c(1,3)], col = colors[1], type = "o", pch = 16, lty = 2 )
lines( data_t[,c(1,4)], col = colors[2], type = "o", pch = 16 )
lines( data_t[,c(1,5)], col = colors[2], type = "o", pch = 16, lty = 2 )
lines( data_t[,c(1,6)], col = colors[3], type = "o", pch = 16 )
lines( data_t[,c(1,7)], col = colors[3], type = "o", pch = 16, lty = 2 )
lines( data_t[,c(1,8)], col = colors[4], type = "o", pch = 16 )
lines( data_t[,c(1,9)], col = colors[4], type = "o", pch = 16, lty = 2 )

legend( "topleft", legend= row_names, fill=legend_colors, lty = rep(c(1, 2), 4), bty = 0 )
dev.off()

# --------------------------
# get distribution of values
print_variations_boxplot <- function( data, title, configurations, range ) {
    par( mar = c( 5, 8.5, 3, 1 ) )
    boxplot( data, main = title, horizontal = T, xlab="Time (seconds)", names = configurations, las=1, log = "x" )
}

reduced_label_no_threads <- unique( apply( config_reduced[-4,], 2, paste, collapse = "," ) )
reduced_label_no_threads <- gsub( pattern = ",", replacement = ", ", x = reduced_label_no_threads)
reduced_label_no_threads <- gsub( pattern = "NO_SIMD, ", replacement = "", x = reduced_label_no_threads)
reduced_label_no_threads <- gsub( pattern = "_", replacement = " ", x = reduced_label_no_threads)
reduced_label_no_threads <- gsub( pattern = "8 b", replacement = "  8 b", x = reduced_label_no_threads)

reduced_label_no_threads_no_64bit <- reduced_label_no_threads[c(-5,-10)]

pdf(file = add_output_dir( "runtime_variation_8_16_bit" ), width = 8, height = 7, pointsize = 12)
par( mfrow = c(2, 1) )
print_variations_boxplot( timing_reduced[,c(1:4,6:9)], "Variation of timing results using 1 thread", reduced_label_no_threads_no_64bit, c(3, 8) )
print_variations_boxplot( timing_reduced[,c(21:24,26:29)], "Variation of timing results using 8 threads", reduced_label_no_threads_no_64bit, c(0.8, 2.5) )
dev.off()

reduced_label_no_threads_64bit <- unique( apply( config_reduced[c(-1,-3),c(1:10,21:30)], 2, paste, collapse = "," ) )
reduced_label_no_threads_64bit <- gsub( pattern = ",", replacement = ", ", x = reduced_label_no_threads_64bit)
reduced_label_no_threads_64bit <- gsub( pattern = "_t", replacement = " threads", x = reduced_label_no_threads_64bit)
reduced_label_no_threads_64bit <- gsub( pattern = "1 threads", replacement = "1 thread  ", x = reduced_label_no_threads_64bit)

pdf(file = add_output_dir( "runtime_variation_64_bit" ), width = 8, height = 3, pointsize = 12)
print_variations_boxplot( timing_reduced[,c(5,10,25,30)], "Variation of timing results in the 64 bit searches", reduced_label_no_threads_64bit, c(25, 135) )
dev.off()

max( apply( timing_reduced[,c(1:10)], 2, sd ) )     # 1 thread
max( apply( timing_reduced[,c(11:20)], 2, sd ) )    # 4 thread
max( apply( timing_reduced[,c(21:30)], 2, sd ) )    # 8 thread


