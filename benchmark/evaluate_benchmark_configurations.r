#install.packages( "ggplot2" )
library(ggplot2)
#library(plyr)
library(reshape2)

#read_reduced_timing( "results/14_02_2015_whole_lib" );
#read_reduced_timing( "results/18_02_2015_whole_lib" );
#read_reduced_timing( "results/19_02_2015_alignment_only", nr_desc_elements = 5 );
read_reduced_timing( "results/30_03_2015_alignment_only", nr_desc_elements = 5 );

total_runtime_func( timing, 5 )

set_indices()

# AVX vs SSE
pdf(file='~/projects/master_thesis/tex/img/runtime_simd_sse_vs_avx.pdf', width = 8, height = 4, pointsize = 12)
par( mfrow = c(1, 2) )
compare_plot_func( idx_avx2, idx_sse41, idx_SW, meantiming, config_reduced[-(1:2),], c("AVX2", "SSE4.1"), "SW calculation: AVX vs. SSE" )
compare_plot_func( idx_avx2, idx_sse41, idx_NW, meantiming, config_reduced[-(1:2),], c("AVX2", "SSE4.1"), "NW calculation: AVX vs. SSE" )
dev.off()

# 8 bit vs 16 bit
compare_plot_func( idx_8bit, idx_16bit, idx_SW, meantiming, config_reduced[-(2:3),], c("8 bit", "16 bit"), "SW calculation: 8 bit vs. 16 bit" )
compare_plot_func( idx_8bit, idx_16bit, idx_NW, meantiming, config_reduced[-(2:3),], c("8 bit", "16 bit"), "NW calculation: 8 bit vs. 16 bit" )

# 1 thread vs 4 threads
compare_plot_func( idx_1t, idx_4t, idx_SW, meantiming, config_reduced[-c(2,4),], c("1 thread", "4 threads"), "SW calculation: 1 thread vs. 4 threads" )
compare_plot_func( idx_1t, idx_4t, idx_NW, meantiming, config_reduced[-c(2,4),], c("1 thread", "4 threads"), "NW calculation: 1 thread vs. 4 threads" )

# 1 thread vs 8 threads
compare_plot_func( idx_1t, idx_8t, idx_SW, meantiming, config_reduced[-c(2,4),], c("1 thread", "8 threads"), "SW calculation: 1 thread vs. 4 threads" )
compare_plot_func( idx_1t, idx_8t, idx_NW, meantiming, config_reduced[-c(2,4),], c("1 thread", "8 threads"), "NW calculation: 1 thread vs. 4 threads" )

# 4 thread vs 8 threads
compare_plot_func( idx_4t, idx_8t, idx_SW, meantiming, config_reduced[-c(2,4),], c("4 thread", "8 threads"), "SW calculation: 1 thread vs. 4 threads" )
compare_plot_func( idx_4t, idx_8t, idx_NW, meantiming, config_reduced[-c(2,4),], c("4 thread", "8 threads"), "NW calculation: 1 thread vs. 4 threads" )

# ------------------
# improvement ratios

# AVX vs. SSE
improvement_per_query <- function( query, timing ) {
    timing_query = timing[ which( timing == query ), -1]
    idx_sse = which( timing_query == "SSE41" )
    idx_avx = which( timing_query == "AVX2" )

    mean_timing_query = apply( timing_query[,-c(1:4)], 1, mean )

    improvements_func( idx_sse, idx_avx, mean_timing_query, config_reduced[-1,], "Improvement: SSE vs. AVX" )
}

improvement_per_query( "O74807", timing )
improvement_per_query( "P19930", timing )
improvement_per_query( "Q3ZAI3", timing )
improvement_per_query( "P18080", timing )





# 8 bit vs 16 bit vs 64 bit
improvements_func( idx_16bit, idx_8bit, meantiming, config_reduced[-3,], "Improvement: 8 bit vs. 16 bit" )

improvements_func( idx_64bit, idx_16bit, meantiming, config_reduced[-c(1,3),], "Improvement: 16 bit vs. 64 bit" ) # TODO incorrect ...

# thread configurations
mean( improvements_func( idx_1t, idx_4t, meantiming, config_reduced[-4,], "Improvement: 1 thread vs. 4 threads" )[,2] )
improvements_func( idx_1t, idx_8t, meantiming, config_reduced[-4,], "Improvement: 1 thread vs. 8 threads" )
improvements_func( idx_4t, idx_8t, meantiming, config_reduced[-4,], "Improvement: 4 thread vs. 8 threads" )

# --------------------------------------------------------
# compare 8 bit to 16 bit, depending on the query sequence
col_names <- c( "O74807", "P19930", "Q3ZAI3", "P18080" )

row_names <- unique( apply( config_reduced[-4,-c(idx_64bit, idx_8t, idx_4t)], 2, paste, collapse = "," ) )
row_names <- gsub( pattern = ",", replacement = ", ", x = row_names)
row_names <- gsub( pattern = "_", replacement = " ", x = row_names)
row_names <- gsub( pattern = "8 b", replacement = "  8 b", x = row_names)
row_names <- gsub( pattern = "SSE41", replacement = "SSE", x = row_names)
row_names <- gsub( pattern = "AVX2", replacement = "AVX", x = row_names)

timing_per_query <- timing[-which( timing == "64_bit", arr.ind=T )[,1],6:15]
avg_per_query <- apply( timing_per_query, 1, mean )

data_t = data.frame( col_names, matrix( avg_per_query, nrow = 4, ncol = 8, byrow = F ), row.names=1 )

n = length( row_names )
colors = rainbow(n, s = 1, v = 1, start = 0, end = max(1, n - 1)/n, alpha = 1)

x = matplot( data_t, type = "b", pch = 1, xlab = "Query sequences", ylab = "Time (seconds)", xlim = c( 1, 4 ), col =colors, lty=1, xaxt = "n" )
axis(1, at = 1:4, labels = col_names, cex.axis = 1)
legend( "topleft", legend= row_names, fill=colors, bty = 0 )

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

pdf(file='~/projects/master_thesis/tex/img/runtime_variation_8_16_bit.pdf', width = 8, height = 7, pointsize = 12)
par( mfrow = c(2, 1) )
print_variations_boxplot( timing_reduced[,c(1:4,6:9)], "Variation of timing results using 1 thread", reduced_label_no_threads_no_64bit )
print_variations_boxplot( timing_reduced[,c(11:14,16:19)], "Variation of timing results using 4 threads", reduced_label_no_threads_no_64bit )
dev.off()

reduced_label_no_threads_64bit <- unique( apply( config_reduced[c(-1,-3),1:20], 2, paste, collapse = "," ) )
reduced_label_no_threads_64bit <- gsub( pattern = ",", replacement = ", ", x = reduced_label_no_threads_64bit)
reduced_label_no_threads_64bit <- gsub( pattern = "_t", replacement = " threads", x = reduced_label_no_threads_64bit)
reduced_label_no_threads_64bit <- gsub( pattern = "1 threads", replacement = "1 thread  ", x = reduced_label_no_threads_64bit)

pdf(file='~/projects/master_thesis/tex/img/runtime_variation_64_bit.pdf', width = 8, height = 3, pointsize = 12)
print_variations_boxplot( timing_reduced[,c(5,10,15,20)], "Variation of timing results in the 64 bit searches", reduced_label_no_threads_64bit )
dev.off()

(maxtiming - mintiming)[c(1:10)]
(maxtiming - mintiming)[c(11:20)]
(maxtiming - mintiming)[c(21:30)]

apply( timing_reduced[,c(1:10)], 2, sd )
apply( timing_reduced[,c(11:20)], 2, sd )
apply( timing_reduced[,c(21:30)], 2, sd )


