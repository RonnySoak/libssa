#install.packages( "ggplot2" )
#library(ggplot2)
#library(plyr)
#library(reshape2)

#read_reduced_timing( "results/14_02_2015_whole_lib" );
#read_reduced_timing( "results/18_02_2015_whole_lib" );
read_reduced_timing( "results/19_02_2015_alignment_only" );

set_indices()

timing_simd = c( totaltiming[ idx_sse41 ], totaltiming[ idx_avx2 ])
total = sum( timing_simd )

total = total * 2 + 8 * total

total / 60 / 60


# AVX vs SSE
compare_plot_func( idx_avx2, idx_sse41, idx_SW, meantiming, config_reduced[-(1:2),], c("AVX2", "SSE4.1"), "SW calculation: AVX vs. SSE" )
compare_plot_func( idx_avx2, idx_sse41, idx_NW, meantiming, config_reduced[-(1:2),], c("AVX2", "SSE4.1"), "NW calculation: AVX vs. SSE" )

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

improvements_func( idx_sse41, idx_avx2, meantiming, config_reduced[-1,], "Improvement: SSE vs. AVX" )
improvements_func( idx_16bit, idx_8bit, meantiming, config_reduced[-3,], "Improvement: 8 bit vs. 16 bit" )
improvements_func( idx_64bit, idx_16bit, meantiming, config_reduced[-c(1,3),], "Improvement: 16 bit vs. 64 bit" ) # TODO incorrect ...
improvements_func( idx_1t, idx_4t, meantiming, config_reduced[-4,], "Improvement: 1 thread vs. 4 threads" )
improvements_func( idx_1t, idx_8t, meantiming, config_reduced[-4,], "Improvement: 1 thread vs. 8 threads" )
improvements_func( idx_4t, idx_8t, meantiming, config_reduced[-4,], "Improvement: 4 thread vs. 8 threads" )

# compare
#
# SSE vs AVX
# 1 thread vs 4 threads
# 8 bit vs 16 bit
#
# old run vs new run
