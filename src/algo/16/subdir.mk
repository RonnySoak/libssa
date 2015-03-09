OBJS += \
./src/algo/16/search_16.o

OBJS_COMPILE_SEPARATE += \
./src/algo/16/16_simd_nw_sse2.o \
./src/algo/16/search_16_util_sse2.o \
./src/algo/16/16_simd_sw_sse2.o \
./src/algo/16/16_simd_nw_avx2.o \
./src/algo/16/16_simd_sw_avx2.o \
./src/algo/16/search_16_util_avx2.o

USER_OBJS += \
./src/algo/16/search_16.h \
./src/algo/16/search_16_util.h

TO_CLEAN +=

src/algo/16/16_simd_nw_sse2.o: src/algo/16/16_simd_nw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse2 -c -o $@ $<
	
src/algo/16/16_simd_sw_sse2.o: src/algo/16/16_simd_sw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse2 -c -o $@ $<
	
src/algo/16/search_16_util_sse2.o: src/algo/16/search_16_util.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse2 -c -o $@ $<

src/algo/16/16_simd_nw_avx2.o: src/algo/16/16_simd_nw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -c -o $@ $<
	
src/algo/16/16_simd_sw_avx2.o: src/algo/16/16_simd_sw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -c -o $@ $<
	
src/algo/16/search_16_util_avx2.o: src/algo/16/search_16_util.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -c -o $@ $<