#install.packages( "ggplot2" )
library(ggplot2)
#library(plyr)
library(reshape2)

source( "scripts/evaluate_config.r" )

read_reduced_timing <- function( file_name, suffix="", nr_desc_elements = 2 ) {
    idx_data = nr_desc_elements + 1

    timing = read_results_csv( file_name )

    configtiming = t(timing[1:(idx_data-1)])
    datatiming = t(timing[idx_data:length(timing[1,])])

    config_labels = apply( configtiming, 2, paste, collapse="," )

    assign( paste( "timing", suffix, sep="" ), timing, envir = .GlobalEnv )

    assign(  "configtiming", configtiming, envir = .GlobalEnv )
    assign(  "config_labels", config_labels, envir = .GlobalEnv )

    assign(  paste( "meantiming", suffix, sep="" ), apply( datatiming, 2, mean ), envir = .GlobalEnv )
}

#read_reduced_timing( "aligner_cmp_05_03_2015" );
read_reduced_timing( "aligner_cmp_01_04_2015" );

query_length <- t(matrix( c( "O74807",110,"P19930",195,"Q3ZAI3",390,"P18080",513 ), nrow=2, ncol=4 ))

row_names = c( "OpAl (AVX2)", "OpAl (SSE4.1)", "SWIPE", "SSW", "libssa (AVX2)", "libssa (SSE4.1)" )
col_names = c( "O74807", "P19930", "Q3ZAI3", "P18080" )

# -------------
# plotting data
data = data.frame( row_names, matrix( meantiming, nrow = 8, ncol = 4, byrow = T )[-c(6,8),], row.names=1 )

names(data) <- query_length[,2]

data_t = cbind( as.integer( query_length[,2] ), matrix( meantiming, nrow = 4, ncol = 8, byrow = F )[,-c(6,8)] )

n = length( row_names )
colors = rainbow(n, s = 1, v = 1, start = 0, end = max(1, n - 1)/n, alpha = 1)
colors[2] = "orange"
colors[3] = "green3"

x_labels <- paste( query_length[,2], query_length[,1], sep = "\n" )

pdf(file = add_output_dir( "aligner_comparison.pdf", width = 10, height = 5, pointsize = 12 )
par( mar=c(5, 5, 3, 2) )
plot( data_t[,c(1,2)], main = "Runtimes of different alignment tools", xlab = "Query length (residues)", ylab = "Time (seconds)",  xlim=c(100,520), ylim= c(2, 50), col = colors[1], type = "o", pch = 16, xaxt = "n", log = "xy" )
axis( 1, at = query_length[,2] )

lines( data_t[,c(1,3)], col = colors[2], type = "o", pch = 16 )
lines( data_t[,c(1,4)], col = colors[3], type = "o", pch = 16 )
lines( data_t[,c(1,5)], col = colors[4], type = "o", pch = 16 )
lines( data_t[,c(1,6)], col = colors[5], type = "o", pch = 16 )
lines( data_t[,c(1,7)], col = colors[6], type = "o", pch = 16 )

legend( "bottomright", legend= row_names, fill=colors, bty = 0 )
dev.off()

# ------------
# improvements
data = data.frame( row_names, matrix( meantiming, nrow = 8, ncol = 4, byrow = T )[-c(6,8),], row.names=1 )

names(data) <- col_names

data_t = data.frame( col_names, matrix( meantiming, nrow = 4, ncol = 8, byrow = F )[,-c(6,8)], row.names=1 )

df_meantiming =  data.frame( row_names, apply( data_t, 2, mean ) )

libssa_avx_8_vs_opal_avx = df_meantiming[1,2] / df_meantiming[5,2]
libssa_sse_8_vs_opal_sse = df_meantiming[2,2] / df_meantiming[6,2]
libssa_sse_8_vs_opal_avx = df_meantiming[1,2] / df_meantiming[6,2]

libssa_sse_8_vs_swipe = df_meantiming[3,2] / df_meantiming[6,2]
libssa_sse_8_vs_ssw = df_meantiming[4,2] / df_meantiming[6,2]

libssa_avx_8_vs_swipe = df_meantiming[3,2] / df_meantiming[5,2]
libssa_avx_8_vs_ssw = df_meantiming[4,2] / df_meantiming[5,2]

opal_avx_vs_sse = df_meantiming[2,2] / df_meantiming[1,2]

libssa_avx_vs_sse = df_meantiming[6,2] / df_meantiming[5,2]



