get_reduced_timing <- function( timing_t, key ) {
    timing_t_reduced = timing_t[,which( timing_t == key, arr.ind=T )[,2]]
    timing_t_reduced = timing_t_reduced[-1,]

    timing_t_reduced = apply (timing_t_reduced, c(1, 2), as.numeric )
    timing_t_reduced = timing_t_reduced[,order(timing_t_reduced[1,])]

    chunk_config_count = 9
    same_config_count = ncol(timing_t_reduced) / chunk_config_count

    data_timing_t = timing_t_reduced[-1,]

    timing_reduced = sapply( seq( 1, ncol(data_timing_t), by=same_config_count ), function(x) rowSums(data_timing_t[,x:((x-1) + same_config_count)]) )

    apply( timing_reduced, c(1, 2), function(x) (x / same_config_count))
}

plot_per_thread_count <- function( timing, thread_key = "1_t", title, yl = c(2, 22) ) {
    timing_t = timing[,which( timing == thread_key, arr.ind=T )[,2]]
    timing_t = timing_t[-2,]

    reduced_timing_rfam = get_reduced_timing( timing_t, "Rfam_11_0" )
    reduced_timing_uniprot = get_reduced_timing( timing_t, "uniprot_sprot" )

    labels = c( 10, 100, 500, 1000, 1500, 2500, 5000, 10000, 25000 )

    data = rbind( apply( reduced_timing_rfam, 2, mean ), apply( reduced_timing_uniprot, 2, mean ) )

    p = plot( labels, data[1,], main = title, type = "o", xlab = "Chunk sizes (number of DB sequences / chunk)", ylab = "Time (seconds)", log = "x", ylim = yl, xaxt="n" )

    axis(side = 1, at=labels, labels=labels )

    lines( labels, data[2,], col="red" )
    points( labels, data[2,], col="red" )

    abline( v = labels[which.min(data[2,])], col="red2", lty=2 )
    abline( h = min(data[2,]), col="red2", lty=2 )
    abline( v = labels[which.min(data[1,])], col="darkgrey", lty=2 )
    abline( h = min(data[1,]), col="darkgrey", lty=2 )

    legend( "topright", legend=c("UniProtKB/Swiss-Prot", "Rfam"), fill=c("black","red" ) )
}

timing = read.csv( file="results/28_02_2015_chunks", header=FALSE, sep="," )
timing = t(timing)
timing = timing[-c(2:5),]

p1 = timing[1:(576*2),]

p2 = p1[which( p1 == "P19930", arr.ind=T )[,1],]
p3 = p1[which( p1 == "Q3ZAI3", arr.ind=T )[,1],]


total_runtime_func( rbind2( p2, p3 ), 7 )

pdf(file='~/projects/master_thesis/tex/img/runtime_avg_chunks_new.pdf', width = 6, height = 10, pointsize = 12)
par( mfrow = c(3, 1) )
plot_per_thread_count( timing, "1_t", "Average runtime using 1 thread", c(2, 22) )
plot_per_thread_count( timing, "4_t", "Average runtime using 4 threads", c(0, 8) )
plot_per_thread_count( timing, "8_t", "Average runtime using 8 threads", c(0, 8) )
dev.off()

