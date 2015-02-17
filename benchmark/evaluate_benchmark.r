nr_desc_elements <- 5
run_count <- 5

names_simd <- c( "AVX2", "SSE4.1" )
namestypes <- c( "SW", "NW" )
namesthreads <- c( "1t", "4t", "8t" )
namesb_width <- c( "8b", "16b", "64b" )
names_runs <- c( "current", "previous" )
names_queries <- c( "O74807.fasta", "P18080.fasta", "P19930.fasta", "Q3ZAI3.fasta" )

idx_data <- nr_desc_elements + 1
idx_last <- nr_desc_elements + run_count

timing <- read.csv(file="results/14_02_15_11_15",header=FALSE,sep=",");

datatiming <- t(timing[idx_data:idx_last])
configtiming <- t(timing[0:(idx_data-1)])

maxtiming <- apply(datatiming,2,max)
mintiming <- apply(datatiming,2,min)
meantiming <- apply(datatiming,2,mean)
mediantiming <- apply(datatiming,2,median)

idx_avx2 <- which( timing == "AVX2", arr.ind=T )[,1]
idx_sse41 <- which( timing == "SSE41", arr.ind=T )[,1]

idx_SW <- which( timing == "SW", arr.ind=T )[,1]
idx_NW <- which( timing == "NW", arr.ind=T )[,1]

idx_8bit <- which( timing == "8b", arr.ind=T )[,1]
idx_16bit <- which( timing == "16b", arr.ind=T )[,1]
idx_64bit <- which( timing == "64b", arr.ind=T )[,1]

idx_1t <- which( timing == "1t", arr.ind=T )[,1]
idx_4t <- which( timing == "4t", arr.ind=T )[,1]
idx_8t <- which( timing == "8t", arr.ind=T )[,1]

avx2_sw <- maxtiming[ duplicated( c(idx_avx2, idx_SW) ) ]
sse41_sw <- maxtiming[ duplicated( c(idx_sse41, idx_SW) ) ]


avx_vs_sse_sw <- matrix( c( avx2_sw, sse41_sw ), ncol=length(avx2_sw), byrow=T )
print( avx_vs_sse_sw )

config_avx_vs_sse = apply( configtiming[-(2:3),], 2, paste, collapse="," )
config_avx_vs_sse = subset( config_avx_vs_sse, subset=configtiming[3, ] == "SW" )
config_avx_vs_sse = config_avx_vs_sse[ duplicated( config_avx_vs_sse ) ]

barplot(avx_vs_sse_sw, main="SW calculation (AVX vs. SSE) by configuration", ylab="Time in seconds", xlab="Configurations", col=c("orange","darkblue"), beside=TRUE, names.arg = config_avx_vs_sse, las=2 )










print(datatiming[ ,duplicated( c(idx_avx2, idx_SW) ) ])

boxplot( x =datatiming[ ,duplicated( c(idx_avx2, idx_SW) ) ], main="Timing data", ylab="Time in seconds", xlab="AVX2 - SW" )


boxplot( x =datatiming, main="Timing data", ylab="Time in seconds", xlab="Configurations" )
boxplot( x =datatiming, main="Timing data", ylab="Time in seconds", xlab="Configurations" )
boxplot( x =datatiming, main="Timing data", ylab="Time in seconds", xlab="Configurations" )
boxplot( x =datatiming, main="Timing data", ylab="Time in seconds", xlab="Configurations" )

print( maxtiming )
print( maxtiming[ idx_avx2 ] )
print( maxtiming[ idx_SW ] )

# compare
#
# SSE vs AVX
# 1 thread vs 4 threads
# 8 bit vs 16 bit
#
# old run vs new run
