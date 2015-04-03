#install.packages( "ggplot2" )
#library(ggplot2)
#library(plyr)
#library(reshape2)

read_reduced_timing( "results/19_02_2015_alignment_only", "_1902" );
read_reduced_timing( "results/18_02_2015_whole_lib", "_1802" );

set_indices()

improvements_func <- function( vdata_a, vdata_b, config_reduced, title ) {
    data = matrix( c( vdata_a, vdata_b ), ncol=length(vdata_a), byrow=T )

    improvement = sapply( seq( 1, length(vdata_a), 1), function(x) (data[1,x] / data[2,x]) )

    x_names = apply( config_reduced, 2, paste, collapse="," )

    improvement = data.frame( x_names, improvement )

    print( title )
    print( improvement )
}

improvements_func( meantiming_1802, meantiming_1902, config_reduced, "Comparison whole lib vs alignment only" )

data = matrix( c( meantiming_1802, meantiming_1902 ), ncol=length(meantiming_1902), byrow=T )
improvement = sapply( seq( 1, length(meantiming_1902), 1), function(x) (data[1,x] / data[2,x]) )

summary( improvement )

data = matrix( c( meantiming_1802, meantiming_1902 ), ncol=length(meantiming_1902), byrow=T )
difference = sapply( seq( 1, length(meantiming_1902), 1), function(x) (data[1,x] - data[2,x]) )

summary( difference )

