# libssa

# Makefile for libssa

# non compilable files, some targets depend on
DEPS := Makefile
# objects to compile
OBJS := 
# files some targets depend on, like header files
USR_OBJS :=
# tests
TESTS := 
# files to clean, that are not in OBJS or TESTS 
TO_CLEAN := libssa.a src/libssa_example.o

COVERAGE_DIR = coverage_data

# All of the sources participating in the build are defined here
-include tests/subdir.mk
-include tests/algo/subdir.mk
-include tests/util/subdir.mk
-include src/subdir.mk
-include src/algo/subdir.mk
-include src/util/subdir.mk

MPI_COMPILE := `mpicxx --showme:compile`
MPI_LINK := `mpicxx --showme:link`

# add "-fprofile-arcs -ftest-coverage" to COMMON for code coverage

LIBS := -lpthread -lm -lsdb 
TEST_LIBS := -lcheck -lrt

# Intel options
#CXX := icpc
#CXXFLAGS := -Wall -Wno-missing-declarations -fast -xSSE2 $(COMMON)

# GNU options
CXX := gcc
CXXFLAGS := -Wall -O0 -std=c99 -march=native --coverage

PROG := libssa libssa_check libssa_example

.SUFFIXES := .o .c

%.o : %.c $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< -L. -lsdb 

all : init $(PROG)

#mpilibssa.o : src/libssa.cc $(OBJS) $(DEPS)
#	$(CXX) $(CXXFLAGS) -DMPIlibssa $(MPI_COMPILE) -c -o $@ src/libssa.cc

init:
	@echo 'Copying file libsdb.a'
	cp ../libsdb/libsdb.a .

libssa : init $(OBJS) $(USR_OBJS) $(DEPS)
	@echo 'Building target: $@'
	ar -cvq libssa.a $(DEPS) $(OBJS)
	@echo 'Finished building target: $@'

#mpilibssa : mpilibssa.o $(OBJS) $(DEPS)
#	@echo 'Building target: $@'
#	$(CXX) $(LINKFLAGS) -o $@ mpilibssa.o $(OBJS) $(LIBS) $(MPI_LINK)
#	@echo 'Finished building target: $@'

libssa_check : init libssa $(TESTS)
	@echo 'Building target: $@'
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(TESTS) $(TEST_LIBS) -L. $(LIBS) 
	@echo 'Finished building target: $@'
	
libssa_example : init libssa ./src/libssa_example.o
	@echo 'Building target: $@'
	$(CXX) $(CXXFLAGS) -o $@ ./src/libssa_example.c -L. -lssa $(LIBS)
	@echo 'Finished building target: $@'

# clean created files
clean:
	-rm -f $(OBJS) $(TESTS) $(TO_CLEAN) $(PROG) libsdb.a gmon.out
	rm -rf $(COVERAGE_DIR)
	find . -type f -name '*.gcda' -print | xargs /bin/rm -f
	find . -type f -name '*.gcno' -print | xargs /bin/rm -f
	find . -type f -name '*.gcov' -print | xargs /bin/rm -f
	
	
# to clean later *.gcov tests/*.gcda tests/*.gcno src/*.gcda src/*.gcno

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
