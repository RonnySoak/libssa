#!/bin/bash
#
# Runs a comand N times and prints the minimum runtime.
# All output of the comand is send to /dev/null
#
# The scripts prints the runtimes, of all calls of the specified comand,
# as a comma separated list.
#
# Param 1: N
# Param 2: the command with all parameters enclosed by ""
#

COUNT=$1
LOG_FILE=$2
CMD=$3

for I in $(seq 1 $COUNT)
do
    START=$(date +%s.%N)
    $CMD > /dev/null
    END=$(date +%s.%N)

    RUNTIME=$(echo "$END - $START" | bc)

	printf ",$RUNTIME" >> $LOG_FILE
	printf ",$RUNTIME"
done
