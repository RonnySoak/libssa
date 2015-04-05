# libssa - benchmarks

RESULT_OUTPUT_DIR = results

DEPS := Makefile src/benchmark_util.c src/benchmark_util.h

CXX := gcc# vtcc -finstrument-functions-exclude-file-list=include

LIBS := -L.. -lssa -pthread -lm -lsdb
BASE_FLAGS := -Wall -O3 -std=c99 -march=native

PROG := wholelib alignmentonly chunks threads pairwise queries

all : init $(PROG)

init :
	@echo 'create result dir'
	mkdir -p $(RESULT_OUTPUT_DIR)

wholelib : init
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ src/benchmark_whole_lib.c src/benchmark_util.c $(LIBS)
	@echo 'Finished building target: $@'

alignmentonly : init
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ src/benchmark_alignment_only.c src/benchmark_util.c $(LIBS)
	@echo 'Finished building target: $@'

threads : init
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ src/benchmark_threads.c src/benchmark_util.c $(LIBS)
	@echo 'Finished building target: $@'

chunks : init
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ src/benchmark_chunks.c src/benchmark_util.c $(LIBS)
	@echo 'Finished building target: $@'

pairwise : init
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ src/benchmark_pairwise_performance.c src/benchmark_util.c $(LIBS)
	@echo 'Finished building target: $@'

queries : init
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ src/benchmark_queries.c src/benchmark_util.c $(LIBS)
	@echo 'Finished building target: $@'

clean:
	rm -f $(PROG)
