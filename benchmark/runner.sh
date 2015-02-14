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
CMD=$2

MIN=0
MAX=0
TOTAL=0

for I in $(seq 1 $COUNT)
do
#    printf "$I . "
    START=$(date +%s.%N)
    $CMD > /dev/null
    END=$(date +%s.%N)

    RUNTIME=$(echo "$END - $START" | bc)
    if [ "$MIN" == "0" ]
    then
		MIN=$RUNTIME
    elif [ $(echo "$RUNTIME < $MIN" | bc) == "1" ]
    then
        MIN=$RUNTIME
    fi

	if [ $(echo "$RUNTIME > $MAX" | bc) == "1" ]
    then
        MAX=$RUNTIME
    fi

	TOTAL=$(echo "$TOTAL + $RUNTIME"|bc)
done

MEAN=$(echo "$TOTAL / $COUNT" | bc -l)

printf "(min/max/mean) time: $MIN \t $MAX \t $MEAN \t\n"
