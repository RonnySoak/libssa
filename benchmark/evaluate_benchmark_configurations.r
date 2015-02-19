#install.packages( "ggplot2" )
#library(ggplot2)
#library(plyr)
#library(reshape2)

nr_desc_elements <- 5

idx_data <- nr_desc_elements + 1

timing <- read.csv(file="results/18_02_2015_whole_lib",header=FALSE,sep=",");
#timing <- read.csv(file="results/14_02_2015_whole_lib",header=FALSE,sep=",");

configtiming <- t(timing[2:(idx_data-1)])
datatiming <- t(timing[idx_data:length(timing[1,])])

config_reduced <- configtiming[, seq( 1, ncol(datatiming), by=4 )]
config_labels <- apply( configtiming, 2, paste, collapse="," )

timing_reduced <- sapply( seq( 1, ncol(datatiming), by=4 ), function(x) rowSums(datatiming[, x:(x+4-1)]) )
timing_reduced <- apply( timing_reduced, c(1, 2), function(x) (x / 4))

maxtiming <- apply(timing_reduced,2,max)
mintiming <- apply(timing_reduced,2,min)
meantiming <- apply(timing_reduced,2,mean)
mediantiming <- apply(timing_reduced,2,median)

idx_avx2 <- which( config_reduced == "AVX2", arr.ind=T )[, 2]
idx_sse41 <- which( config_reduced == "SSE41", arr.ind=T )[,2]
idx_nosimd <- which( config_reduced == "NO_SIMD", arr.ind=T )[,2]

idx_SW <- which( config_reduced == "SW", arr.ind=T )[,2]
idx_NW <- which( config_reduced == "NW", arr.ind=T )[,2]

idx_8bit <- which( config_reduced == "8_b", arr.ind=T )[,2]
idx_16bit <- which( config_reduced == "16_b", arr.ind=T )[,2]
idx_64bit <- which( config_reduced == "64_b", arr.ind=T )[,2]

idx_1t <- which( config_reduced == "1_t", arr.ind=T )[,2]
idx_4t <- which( config_reduced == "4_t", arr.ind=T )[,2]
idx_8t <- which( config_reduced == "8_t", arr.ind=T )[,2]

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

    barplot( plot_data, main=title, ylab="Time in seconds", xlab="Configurations", col=c("orange","darkblue"), beside=T, names.arg = x_names, ylim=c(0, 20), las=1, space=c(0.2,0.8) )

    legend( "topright", legend=legend_text, fill=c("orange","darkblue" ), bty = 0 )
}

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

# improvements
improvements_func <- function( idx_a, idx_b, vdata, config_reduced, title ) {
    data = matrix( c( vdata[ idx_a ], vdata[ idx_b ] ), ncol=length(idx_a), byrow=T )

    improvement = sapply( seq( 1, length(idx_a), 1), function(x) (data[1,x] / data[2,x]) )

    x_names = get_x_labels( idx_a, idx_b, config_reduced )

    improvement = data.frame( x_names, improvement )

    print( title )
    print( improvement )
}

improvements_func( idx_sse41, idx_avx2, meantiming, config_reduced[-1,], "Improvement: SSE vs. AVX" )
improvements_func( idx_16bit, idx_8bit, meantiming, config_reduced[-3,], "Improvement: 8 bit vs. 16 bit" )
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
