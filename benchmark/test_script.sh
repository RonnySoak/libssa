#!/bin/bash
#
# Runs the benchmark of libssa, including database loading, in different configurations.
#

COUNT=3

DATABASE="uniprot_sprot.fasta"
GAP_O=3
GAP_E=1
MATRIX="BLOSUM50"

declare -a THREADS_ARR=("1" "4")
declare -a TYPE_ARR=("SW" "NW")
declare -a SIMD_ARR=("SSE41" "AVX2")
declare -a BIT_ARR=("8" "16", "64")
declare -a QUERY_ARR=("O74807.fasta" "P18080.fasta" "P19930.fasta" "Q3ZAI3.fasta")

for THREADS in "${THREADS_ARR[@]}"
do
	echo "using $THREADS threads"

	for TYPE in "${TYPE_ARR[@]}"
	do
		for SIMD in "${SIMD_ARR[@]}"
		do
			for BIT_WIDTH in "${BIT_ARR[@]}"
			do
				for QUERY in "${QUERY_ARR[@]}"
				do
					printf "run tests ($QUERY $SIMD, $TYPE, $BIT_WIDTH bit)\t...\t"
					./runner.sh $COUNT "./benchmark -N $THREADS -O $GAP_O -E $GAP_E -M $MATRIX -c 10 -i data/$QUERY -d data/$DATABASE -t $TYPE -b $BIT_WIDTH -s $SIMD"
				done
				echo ""
			done
		done
	done
done

