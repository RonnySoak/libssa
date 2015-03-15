OBJS_COMPILE_SEPARATE += \
./src/algo/simd/8_simd_nw_sse41.o \
./src/algo/simd/8_simd_sw_sse41.o \
./src/algo/simd/8_simd_nw_avx2.o \
./src/algo/simd/8_simd_sw_avx2.o \
./src/algo/simd/16_simd_nw_sse2.o \
./src/algo/simd/16_simd_sw_sse2.o \
./src/algo/simd/16_simd_nw_avx2.o \
./src/algo/simd/16_simd_sw_avx2.o

src/algo/simd/8_simd_nw_sse41.o: src/algo/simd/search_simd_nw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse4.1 -DSEARCH_8_BIT -c -o $@ $<
	
src/algo/simd/8_simd_sw_sse41.o: src/algo/simd/search_simd_sw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse4.1 -DSEARCH_8_BIT -c -o $@ $<

src/algo/simd/8_simd_nw_avx2.o: src/algo/simd/search_simd_nw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -DSEARCH_8_BIT -c -o $@ $<
	
src/algo/simd/8_simd_sw_avx2.o: src/algo/simd/search_simd_sw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -DSEARCH_8_BIT -c -o $@ $<

src/algo/simd/16_simd_nw_sse2.o: src/algo/simd/search_simd_nw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse2 -c -o $@ $<
	
src/algo/simd/16_simd_sw_sse2.o: src/algo/simd/search_simd_sw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -msse2 -c -o $@ $<

src/algo/simd/16_simd_nw_avx2.o: src/algo/simd/search_simd_nw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -c -o $@ $<
	
src/algo/simd/16_simd_sw_avx2.o: src/algo/simd/search_simd_sw.c $(DEPS)
	$(CXX) $(CXXFLAGS) -mavx2 -c -o $@ $<
