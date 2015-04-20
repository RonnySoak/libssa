#!/bin/bash
#
# Runs all the alignment tools, in different configurations, collecting the wall clock times.
#

COUNT=10
LOG_FILE="aligner_cmp_$(date +%d.%m.%y_%H_%M)"

# remove old log file first
rm $LOG_FILE 2> /dev/null

DATABASE="uniprot_sprot.fasta"
GAP_O=3
GAP_E=1
MATRIX="BLOSUM50"

declare -a ALIGNER_ARR=(
	"opal_avx2"
	"opal_sse41"
	"swipe"
	"ssw"
	"libssa_avx2_8"
	"libssa_avx2_16"
	"libssa_sse41_8"
	"libssa_sse41_16" )

declare -a QUERY_ARR=("O74807" "P19930" "Q3ZAI3" "P18080")

for ALIGNER in "${ALIGNER_ARR[@]}"
do
	for QUERY in "${QUERY_ARR[@]}"
	do
		printf "$ALIGNER,$QUERY" >> $LOG_FILE
		printf "$ALIGNER,$QUERY"

		./runner.sh $COUNT $LOG_FILE "./run_$ALIGNER.sh ../data/$QUERY $DATABASE $GAP_O $GAP_E $MATRIX"

		printf "\n" >> $LOG_FILE
		printf "\n"
	done
done
