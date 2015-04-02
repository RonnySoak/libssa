# libssa - benchmarks

RESULT_OUTPUT_DIR = results

DEPS := Makefile

CXX := gcc# vtcc -finstrument-functions-exclude-file-list=include

LIBS := -L.. -lssa -pthread -lm -lsdb
BASE_FLAGS := -Wall -O3 -std=c99 -march=native

PROG := wholelib alignmentonly chunks threads pairwise

all : init $(PROG)

init :
	@echo 'create result dir'
	mkdir -p $(RESULT_OUTPUT_DIR)

wholelib : init benchmark_whole_lib.c $(DEPS)
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ benchmark_whole_lib.c $(LIBS)
	@echo 'Finished building target: $@'

alignmentonly : init benchmark_alignment_only.c $(DEPS)
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ benchmark_alignment_only.c $(LIBS)
	@echo 'Finished building target: $@'

threads : init benchmark_threads.c $(DEPS)
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ benchmark_threads.c $(LIBS)
	@echo 'Finished building target: $@'

chunks : init benchmark_chunks.c $(DEPS)
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ benchmark_chunks.c $(LIBS)
	@echo 'Finished building target: $@'

pairwise : init benchmark_pairwise_performance.c $(DEPS)
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ benchmark_pairwise_performance.c $(LIBS)
	@echo 'Finished building target: $@'

clean:
	rm -f $(PROG)
