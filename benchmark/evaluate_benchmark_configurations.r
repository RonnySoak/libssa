#install.packages( "ggplot2" )
#library(ggplot2)
#library(plyr)
#library(reshape2)

#read_reduced_timing( "results/14_02_2015_whole_lib" );
#read_reduced_timing( "results/18_02_2015_whole_lib" );
read_reduced_timing( "results/19_02_2015_alignment_only", nr_desc_elements = 5 );

set_indices()

# AVX vs SSE
pdf(file='~/projects/master_thesis/tex/img/runtime_simd_sse_vs_avx_new.pdf', width = 8, height = 4, pointsize = 12)
par( mfrow = c(1, 2) )
compare_plot_func( idx_avx2, idx_sse41, idx_SW, meantiming, configtiming[-(1:2),], c("AVX2", "SSE4.1"), "SW calculation: AVX vs. SSE" )
compare_plot_func( idx_avx2, idx_sse41, idx_NW, meantiming, configtiming[-(1:2),], c("AVX2", "SSE4.1"), "NW calculation: AVX vs. SSE" )
dev.off()

# 8 bit vs 16 bit
compare_plot_func( idx_8bit, idx_16bit, idx_SW, meantiming, configtiming[-(2:3),], c("8 bit", "16 bit"), "SW calculation: 8 bit vs. 16 bit" )
compare_plot_func( idx_8bit, idx_16bit, idx_NW, meantiming, configtiming[-(2:3),], c("8 bit", "16 bit"), "NW calculation: 8 bit vs. 16 bit" )

# 1 thread vs 4 threads
compare_plot_func( idx_1t, idx_4t, idx_SW, meantiming, configtiming[-c(2,4),], c("1 thread", "4 threads"), "SW calculation: 1 thread vs. 4 threads" )
compare_plot_func( idx_1t, idx_4t, idx_NW, meantiming, configtiming[-c(2,4),], c("1 thread", "4 threads"), "NW calculation: 1 thread vs. 4 threads" )

# 1 thread vs 8 threads
compare_plot_func( idx_1t, idx_8t, idx_SW, meantiming, configtiming[-c(2,4),], c("1 thread", "8 threads"), "SW calculation: 1 thread vs. 4 threads" )
compare_plot_func( idx_1t, idx_8t, idx_NW, meantiming, configtiming[-c(2,4),], c("1 thread", "8 threads"), "NW calculation: 1 thread vs. 4 threads" )

# 4 thread vs 8 threads
compare_plot_func( idx_4t, idx_8t, idx_SW, meantiming, configtiming[-c(2,4),], c("4 thread", "8 threads"), "SW calculation: 1 thread vs. 4 threads" )
compare_plot_func( idx_4t, idx_8t, idx_NW, meantiming, configtiming[-c(2,4),], c("4 thread", "8 threads"), "NW calculation: 1 thread vs. 4 threads" )

improvements_func( idx_sse41, idx_avx2, meantiming, configtiming[-1,], "Improvement: SSE vs. AVX" )
improvements_func( idx_16bit, idx_8bit, meantiming, configtiming[-3,], "Improvement: 8 bit vs. 16 bit" )
improvements_func( idx_64bit, idx_16bit, meantiming, configtiming[-c(1,3),], "Improvement: 16 bit vs. 64 bit" ) # TODO incorrect ...
improvements_func( idx_1t, idx_4t, meantiming, configtiming[-4,], "Improvement: 1 thread vs. 4 threads" )
improvements_func( idx_1t, idx_8t, meantiming, configtiming[-4,], "Improvement: 1 thread vs. 8 threads" )
improvements_func( idx_4t, idx_8t, meantiming, configtiming[-4,], "Improvement: 4 thread vs. 8 threads" )

# get distribution of values
print_variations_boxplot <- function( data, title, configurations ) {
    par( mar = c( 5, 8.5, 3, 1 ) )
    boxplot( data, main = title, horizontal = T, xlab="Time (seconds)", names = configurations, las=1 )
}

reduced_label_no_threads <- unique( apply( configtiming[-4,], 2, paste, collapse = "," ) )
reduced_label_no_threads <- gsub( pattern = ",", replacement = ", ", x = reduced_label_no_threads)
reduced_label_no_threads <- gsub( pattern = "NO_SIMD, ", replacement = "", x = reduced_label_no_threads)
reduced_label_no_threads <- gsub( pattern = "_", replacement = " ", x = reduced_label_no_threads)

min_max <- cbind( c( "1t", "4t", "8t" ), matrix( maxtiming - mintiming, nrow=3, ncol=10, byrow = T ) )
min_max <- rbind( c( NA, reduced_label_no_threads ), min_max )

pdf(file='~/projects/master_thesis/tex/img/runtime_variation_new.pdf', width = 6, height = 10, pointsize = 12)
par( mfrow = c(3, 1) )
print_variations_boxplot( timing_reduced[,1:10], "Variation of timing results using 1 thread", reduced_label_no_threads )
print_variations_boxplot( timing_reduced[,11:20], "Variation of timing results using 4 threads", reduced_label_no_threads )
print_variations_boxplot( timing_reduced[,21:30], "Variation of timing results using 8 threads", reduced_label_no_threads )
dev.off()

par( mar = c( 5, 5, 3, 1 ) )
boxplot( t(matrix( maxtiming - mintiming, nrow=3, ncol=10, byrow = T )), horizontal = T, names = c( "1 thread", "4 threads", "8 threads" ), main = "Variation of timing result ranges", las = 1, xlab="Time (seconds)" )

apply( timing_reduced[,1:10], 2, sd )
apply( timing_reduced[,11:20], 2, sd )
apply( timing_reduced[,21:30], 2, sd )

# compare
#
# SSE vs AVX
# 1 thread vs 4 threads
# 8 bit vs 16 bit
#
# old run vs new run
