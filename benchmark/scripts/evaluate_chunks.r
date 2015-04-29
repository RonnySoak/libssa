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

source( "scripts/evaluate_config.r" )

get_chunk_timing <- function( timing_t, key ) {
    timing_t_reduced = timing_t[,which( timing_t == key, arr.ind=T )[,2]]
    timing_t_reduced = timing_t_reduced[-1,]

    timing_t_reduced = apply (timing_t_reduced, c(1, 2), as.numeric )
    timing_t_reduced = timing_t_reduced[,order(timing_t_reduced[1,])]

    chunk_config_count = 11
    same_config_count = ncol(timing_t_reduced) / chunk_config_count

    data_timing_t = timing_t_reduced[-1,]

    timing_reduced = sapply( seq( 1, ncol(data_timing_t), by=same_config_count ), function(x) rowSums(data_timing_t[,x:((x-1) + same_config_count)]) )

    apply( timing_reduced, c(1, 2), function(x) (x / same_config_count))
}

plot_per_thread_count <- function( timing, thread_key = "1_t", title, yl = c(2, 22) ) {
    timing_t = timing[,which( timing == thread_key, arr.ind=T )[,2]]
    timing_t = timing_t[-2,]

    reduced_timing_rfam = get_chunk_timing( timing_t, "Rfam_11_0" )
    reduced_timing_uniprot = get_chunk_timing( timing_t, "uniprot_sprot" )

    labels = c( 10, 100, 500, 1000, 1500, 2500, 5000, 10000, 25000, 50000, 100000 )

    data = rbind( apply( reduced_timing_rfam, 2, mean ), apply( reduced_timing_uniprot, 2, mean ) )

    print(  "raw mean data:" )
    print( data )

    print( "Range (min/max) (1. Rfam, 2. Uniprot): " )
    if( thread_key == "1_t" ) {cut_out <- c(1,2,8,9,10,11)} else {cut_out <- c(1,2,8,9,10,11)}

    data_r = data[1,-cut_out]
    data_u = data[2,-cut_out]

    print( max(data_r) - min(data_r) )
    print( max(data_u) - min(data_u) )

    p = plot( labels, data[1,], main = title, type = "o", pch = 16, xlab = "Chunk sizes (number of DB sequences / chunk)", ylab = "Time (seconds)", log = "xy", ylim = yl, xaxt="n", col = "blue" )

    axis(side = 1, at=labels, labels=labels, options(scipen=5) )

    lines( labels, data[2,], col="orange", pch = 16, type = "o" )

    abline( v = labels[which.min(data[2,])], col="orange", lty=2 )
    abline( h = min(data[2,]), col="orange", lty=2 )
    abline( v = labels[which.min(data[1,])], col="deepskyblue3", lty=2 )
    abline( h = min(data[1,]), col="deepskyblue3", lty=2 )

    legend( "topright", legend=c("UniProtKB/Swiss-Prot", "Rfam"), fill=c("orange", "blue"), bg="white" )
}

#timing <- read_results_csv( "28_02_2015_chunks" )
# timing <- read_results_csv( "04_04_2015_chunks" )
# timing <- read_results_csv( "05_04_2015_chunks" )
timing <- read_results_csv( "06_04_2015_chunks" )

total_runtime_func( timing, 6 )

timing <- timing[ which( timing == "8_bit", arr.ind = T )[,1], ]

timing <- t(timing)
timing <- timing[-c(2:4),]

pdf(file= add_output_dir( "runtime_avg_chunks" ), width = 9, height = 9, pointsize = 12)
par( mfrow = c(2, 1) )
plot_per_thread_count( timing, "1_t", "Average runtime using 1 thread", c(0.8, 20) )
plot_per_thread_count( timing, "8_t", "Average runtime using 8 threads", c(0.25, 6) )
dev.off()



