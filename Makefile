# libssa

# Makefile for libssa

# non compilable files, some targets depend on
DEPS := Makefile
# objects to compile
OBJS := 
OBJS_COMPILE_SEPARATE :=

# files some targets depend on, like header files
USR_OBJS :=
# tests
TESTS := 
# files to clean, that are not in OBJS or TESTS 
TO_CLEAN := libssa.a src/libssa_example.o

COVERAGE_DIR = coverage_data
DEBUG_OUTPUT_DIR = debug_output

# All of the sources participating in the build are defined here
-include tests/subdir.mk
-include tests/algo/subdir.mk
-include tests/algo/8/subdir.mk
-include tests/algo/16/subdir.mk
-include tests/algo/64/subdir.mk
-include tests/util/subdir.mk
-include src/subdir.mk
-include src/algo/subdir.mk
-include src/algo/simd/subdir.mk
-include src/algo/8/subdir.mk
-include src/algo/16/subdir.mk
-include src/algo/64/subdir.mk
-include src/util/subdir.mk

MPI_COMPILE := `mpicxx --showme:compile`
MPI_LINK := `mpicxx --showme:link`

# add "-fprofile-arcs -ftest-coverage" to COMMON for code coverage

#### TODO remove from PROD code ####
#DEBUG_LIBS := -lefence
#DEBUG_FLAGS := -g# --coverage

LIBS := -pthread -lm -lsdb $(DEBUG_LIBS)
TEST_LIBS := -lcheck -lrt

# GNU options
CXX := gcc #vtcc -finstrument-functions-exclude-file-list=include
	
BASE_FLAGS := -Wall -O3 -std=c99 $(DEBUG_FLAGS)

PROG := libssa libssa_check libssa_example

OBJS_ALL := $(OBJS) $(OBJS_COMPILE_SEPARATE)

OBJS_BASE_COMPILE := $(OBJS) $(TESTS) src/libssa_example.o

$(OBJS_BASE_COMPILE): CXXFLAGS := $(BASE_FLAGS) -march=native
$(OBJS_COMPILE_SEPARATE): CXXFLAGS := $(BASE_FLAGS)

# compiles all files without linking
$(OBJS_BASE_COMPILE): %.o : %.c $(DEPS) # TODO add flag -S to get assembly code, instead of -c and change filename to .s
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all : init $(PROG)

init:
	@echo 'Copying file libsdb.a'
	cp ../libsdb/libsdb.a .
	mkdir -p $(DEBUG_OUTPUT_DIR)

libssa : init $(OBJS_ALL) $(USR_OBJS) $(DEPS)
	@echo 'Building target: $@'
	ar -cvq libssa.a $(DEPS) $(OBJS_ALL)
	@echo 'Finished building target: $@'

libssa_check : init libssa $(TESTS)
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ $(OBJS_ALL) $(TESTS) $(TEST_LIBS) -L. $(LIBS) 
	@echo 'Finished building target: $@'
	
libssa_example : init libssa ./src/libssa_example.o
	@echo 'Building target: $@'
	$(CXX) $(BASE_FLAGS) -march=native -o $@ ./src/libssa_example.c -L. -lssa $(LIBS)
	@echo 'Finished building target: $@'

# clean created files
clean:
	rm -f $(OBJS_ALL) $(TESTS) $(TO_CLEAN) $(PROG) libsdb.a gmon.out
	rm -rf $(COVERAGE_DIR)
	rm -rf $(DEBUG_OUTPUT_DIR)
	find . -type f -name '*.gcda' -print | xargs /bin/rm -f
	find . -type f -name '*.gcno' -print | xargs /bin/rm -f
	find . -type f -name '*.gcov' -print | xargs /bin/rm -f

# run tests
check:
	./libssa_check

test_coverage :
	lcov --directory . -z	# reset gcov counters
	@echo Running tests
	./libssa_check
	@echo Computing test coverage
	mkdir -p $(COVERAGE_DIR)
	lcov --directory . --capture --output-file $(COVERAGE_DIR)/libssa_tests.info
	genhtml --output-directory $(COVERAGE_DIR)/cov_tests $(COVERAGE_DIR)/libssa_tests.info
	@echo Finished computing test coverage

example_coverage :
	lcov --directory . -z	# reset gcov counters
	@echo Running example
	./example
	@echo Computing test coverage
	mkdir -p $(COVERAGE_DIR)
	lcov --directory . --capture --output-file $(COVERAGE_DIR)/libssa_example.info
	genhtml --output-directory $(COVERAGE_DIR)/cov_example $(COVERAGE_DIR)/libssa_example.info
	@echo Finished computing test coverage

# run example
example:
	./libssa_example
