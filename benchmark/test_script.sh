#!/bin/bash
#
# Runs the benchmark of libssa, including database loading, in different configurations.
#

COUNT=10
LOG_FILE=results/18_02_2015_whole_lib

DATABASE="uniprot_sprot.fasta"
GAP_O=3
GAP_E=1
MATRIX="BLOSUM50"

declare -a THREADS_ARR=("1" "4" "8")
declare -a TYPE_ARR=("SW" "NW")
declare -a SIMD_ARR=("SSE41" "AVX2")
declare -a BIT_ARR=("8" "16")
declare -a QUERY_ARR=("O74807" "P18080" "P19930" "Q3ZAI3")

for THREADS in "${THREADS_ARR[@]}"
do
	for TYPE in "${TYPE_ARR[@]}"
	do
		for SIMD in "${SIMD_ARR[@]}"
		do
			for BIT_WIDTH in "${BIT_ARR[@]}"
			do
				for QUERY in "${QUERY_ARR[@]}"
				do
					printf "$QUERY,$SIMD,$TYPE,$BIT_WIDTH b,$THREADS t," >> $LOG_FILE
					printf "$QUERY,$SIMD,$TYPE,$BIT_WIDTH b,$THREADS t,"
					./runner.sh $COUNT $LOG_FILE "./benchmark -N $THREADS -O $GAP_O -E $GAP_E -M $MATRIX -c 10 -i data/$QUERY.fasta -d data/$DATABASE -t $TYPE -b $BIT_WIDTH -s $SIMD"
					printf "\n" >> $LOG_FILE
					printf "\n"
				done
			done
		done

		# 64 bit
		for QUERY in "${QUERY_ARR[@]}"
		do
			printf "$QUERY,NO_SIMD,$TYPE,64_b,$THREADS t" >> $LOG_FILE
			printf "$QUERY,NO_SIMD,$TYPE,64_b,$THREADS t"
			./runner.sh $COUNT $LOG_FILE "./benchmark -N $THREADS -O $GAP_O -E $GAP_E -M $MATRIX -c 10 -i data/$QUERY.fasta -d data/$DATABASE -t $TYPE -b 64 -s $SIMD"
			printf "\n" >> $LOG_FILE
			printf "\n"
		done
	done
done

echo "do benchmark 2"
./benchmark_2
