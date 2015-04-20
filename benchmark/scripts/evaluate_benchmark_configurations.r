#install.packages( "ggplot2" )
library(ggplot2)
#library(plyr)
library(reshape2)

source( "scripts/evaluate_config.r" )
source( "scripts/evaluate_functions.r" )

#read_reduced_timing( "14_02_2015_whole_lib" );
#read_reduced_timing( "18_02_2015_whole_lib" );
#read_reduced_timing( "19_02_2015_alignment_only", nr_desc_elements = 5 );
read_reduced_timing( "30_03_2015_alignment_only", nr_desc_elements = 5 );

total_runtime_func( timing, 5 )

set_indices()

# AVX vs SSE
pdf(file = add_output_dir( "runtime_simd_sse_vs_avx" ), width = 8, height = 4, pointsize = 12)
par( mfrow = c(1, 2) )
compare_plot_func( idx_avx2, idx_sse41, idx_SW, meantiming, config_reduced[-(1:2),], c("AVX2", "SSE4.1"), "SW calculation: AVX vs. SSE" )
compare_plot_func( idx_avx2, idx_sse41, idx_NW, meantiming, config_reduced[-(1:2),], c("AVX2", "SSE4.1"), "NW calculation: AVX vs. SSE" )
dev.off()

# ------------------
# improvement ratios
improvement_per_query <- function( query, timing, key1, key2, config_line ) {
    timing_query = timing[ which( timing == query ), -1]
    timing_query = timing_query[ which( timing_query == "1_t", arr.ind = T )[,1],]

    idx_1 = which( timing_query == key1, arr.ind = T )[,1]
    idx_2 = which( timing_query == key2, arr.ind = T )[,1]

    mean_timing_query = apply( timing_query[,-c(1:4)], 1, mean )

    data = matrix( c( mean_timing_query[ idx_1 ], mean_timing_query[ idx_2 ] ), ncol=length(idx_1), byrow=T )

    improvement = sapply( seq( 1, length(idx_1), 1), function(x) (data[1,x] / data[2,x]) )

    x_names = get_x_labels( idx_1, idx_2, config_reduced[-c(config_line,4),] )

    improvement = data.frame( x_names, improvement )

    print( "Improvement:" )
    print( improvement )
    print( "mean:" )
    print( mean( improvement[,2] ) )
}

# AVX vs. SSE
improvement_per_query( "O74807", timing, "SSE41", "AVX2", 1 )
improvement_per_query( "P19930", timing, "SSE41", "AVX2", 1 )
improvement_per_query( "Q3ZAI3", timing, "SSE41", "AVX2", 1 )
improvement_per_query( "P18080", timing, "SSE41", "AVX2", 1 )

# 8 vs. 16 bit
improvement_per_query( "O74807", timing, "16_bit", "8_bit", 3 )
improvement_per_query( "P19930", timing, "16_bit", "8_bit", 3 )
improvement_per_query( "Q3ZAI3", timing, "16_bit", "8_bit", 3 )
improvement_per_query( "P18080", timing, "16_bit", "8_bit", 3 )


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
print_variations_boxplot <- function( data, title, configurations ) {
    par( mar = c( 5, 8.5, 3, 1 ) )
    boxplot( data, main = title, horizontal = T, xlab="Time (seconds)", names = configurations, las=1 )
}

reduced_label_no_threads <- unique( apply( config_reduced[-4,], 2, paste, collapse = "," ) )
reduced_label_no_threads <- gsub( pattern = ",", replacement = ", ", x = reduced_label_no_threads)
reduced_label_no_threads <- gsub( pattern = "NO_SIMD, ", replacement = "", x = reduced_label_no_threads)
reduced_label_no_threads <- gsub( pattern = "_", replacement = " ", x = reduced_label_no_threads)
reduced_label_no_threads <- gsub( pattern = "8 b", replacement = "  8 b", x = reduced_label_no_threads)

reduced_label_no_threads_no_64bit <- reduced_label_no_threads[c(-5,-10)]

pdf(file = add_output_dir( "runtime_variation_8_16_bit" ), width = 8, height = 7, pointsize = 12)
par( mfrow = c(2, 1) )
print_variations_boxplot( timing_reduced[,c(1:4,6:9)], "Variation of timing results using 1 thread", reduced_label_no_threads_no_64bit )
print_variations_boxplot( timing_reduced[,c(21:24,26:29)], "Variation of timing results using 8 threads", reduced_label_no_threads_no_64bit )
dev.off()

reduced_label_no_threads_64bit <- unique( apply( config_reduced[c(-1,-3),c(1:10,21:30)], 2, paste, collapse = "," ) )
reduced_label_no_threads_64bit <- gsub( pattern = ",", replacement = ", ", x = reduced_label_no_threads_64bit)
reduced_label_no_threads_64bit <- gsub( pattern = "_t", replacement = " threads", x = reduced_label_no_threads_64bit)
reduced_label_no_threads_64bit <- gsub( pattern = "1 threads", replacement = "1 thread  ", x = reduced_label_no_threads_64bit)

pdf(file = add_output_dir( "runtime_variation_64_bit" ), width = 8, height = 3, pointsize = 12)
print_variations_boxplot( timing_reduced[,c(5,10,25,30)], "Variation of timing results in the 64 bit searches", reduced_label_no_threads_64bit )
dev.off()

(maxtiming - mintiming)[c(1:10)]
(maxtiming - mintiming)[c(11:20)]
(maxtiming - mintiming)[c(21:30)]

apply( timing_reduced[,c(1:10)], 2, sd )
apply( timing_reduced[,c(11:20)], 2, sd )
apply( timing_reduced[,c(21:30)], 2, sd )


