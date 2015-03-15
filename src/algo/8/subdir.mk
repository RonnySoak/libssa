OBJS += \
./src/algo/8/search_8.o \

OBJS_COMPILE_SEPARATE += \
./src/algo/8/search_8_util_sse41.o \
./src/algo/8/search_8_util_avx2.o

USER_OBJS += \
./src/algo/8/search_8.h \
./src/algo/8/search_8_util.h

TO_CLEAN +=

src/algo/8/search_8_util_sse41.o: src/algo/8/search_8_util.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse4.1 -c -o $@ $<
	
src/algo/8/search_8_util_avx2.o: src/algo/8/search_8_util.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -c -o $@ $<