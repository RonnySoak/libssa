OBJS += \
./src/algo/8/search_8.o \

OBJS_COMPILE_SEPARATE += \
./src/algo/8/8_simd_nw_sse41.o \
./src/algo/8/search_8_util_sse41.o \
./src/algo/8/8_simd_sw_sse41.o \
./src/algo/8/8_simd_nw_avx2.o \
./src/algo/8/8_simd_sw_avx2.o \
./src/algo/8/search_8_util_avx2.o

USER_OBJS += \
./src/algo/8/search_8.h \
./src/algo/8/search_8_util.h

TO_CLEAN +=

src/algo/8/8_simd_nw_sse41.o: src/algo/8/8_simd_nw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse4.1 -c -o $@ $<
	
src/algo/8/8_simd_sw_sse41.o: src/algo/8/8_simd_sw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse4.1 -c -o $@ $<
	
src/algo/8/search_8_util_sse41.o: src/algo/8/search_8_util.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse4.1 -c -o $@ $<

src/algo/8/8_simd_nw_avx2.o: src/algo/8/8_simd_nw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -D__AVX2__ -c -o $@ $<
	
src/algo/8/8_simd_sw_avx2.o: src/algo/8/8_simd_sw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -D__AVX2__ -c -o $@ $<
	
src/algo/8/search_8_util_avx2.o: src/algo/8/search_8_util.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -D__AVX2__ -c -o $@ $<