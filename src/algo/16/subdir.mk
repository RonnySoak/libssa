OBJS += \
./src/algo/16/search_16.o \
./src/algo/16/16_simd_sse2_nw.o \
./src/algo/16/16_simd_sse2_sw.o \
./src/algo/16/16_simd_avx2_nw.o \
./src/algo/16/16_simd_avx2_sw.o \
./src/algo/16/search_16_sse2_util.o \
./src/algo/16/search_16_avx2_util.o

USER_OBJS += \
./src/algo/16/search_16.h \
./src/algo/16/search_16_util.h

TO_CLEAN +=