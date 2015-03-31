#install.packages( "ggplot2" )
library(ggplot2)
#library(plyr)
library(reshape2)

read_reduced_timing <- function( file_name, suffix="", element_to_remove = 2, nr_desc_elements = 2 ) {
    idx_data = nr_desc_elements + 1

    timing = read.csv( file=file_name, header=FALSE, sep="," )

    configtiming = t(timing[1:(idx_data-1)])
    datatiming = t(timing[idx_data:length(timing[1,])])

    config_labels = apply( configtiming, 2, paste, collapse="," )

    assign( paste( "timing", suffix, sep="" ), timing, envir = .GlobalEnv )

    assign(  "configtiming", configtiming, envir = .GlobalEnv )
    assign(  "config_labels", config_labels, envir = .GlobalEnv )

    assign(  paste( "meantiming", suffix, sep="" ), apply( datatiming, 2, mean ), envir = .GlobalEnv )
}

read_reduced_timing( "results/30_03_2015_threads" );

element_to_remove = 2
nr_desc_elements = 3
idx_data = nr_desc_elements + 1

timing = read.csv( file="results/30_03_2015_threads", header=FALSE, sep="," )
timing = timing[,c(-1,-2)]

# get distribution of values
print_variations_boxplot <- function( data, title, configurations ) {
    par( mar = c( 5, 10, 3, 1 ) )
    boxplot( data, main = title, horizontal = T, xlab = "Time (seconds)", names = configurations, las = 1 )
}

length = length( timing[,1] )
no_simd_idx = seq(3, length, 3)

reduced_labels <- unique( apply( timing[-no_simd_idx,1:3], 1, paste, collapse = "," ) )
reduced_labels <- gsub( pattern = ",", replacement = ", ", x = reduced_labels)
reduced_labels <- gsub( pattern = "_t", replacement = " threads", x = reduced_labels)
reduced_labels <- gsub( pattern = "_", replacement = " ", x = reduced_labels)
reduced_labels <- gsub( pattern = "8 b", replacement = " 8 b", x = reduced_labels)
reduced_labels <- gsub( pattern = "1 threads", replacement = "1 thread  ", x = reduced_labels)

print_variations_boxplot( t(timing[-no_simd_idx,4:13]), "Runtime variation per thread count", reduced_labels )

