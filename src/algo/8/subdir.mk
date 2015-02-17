OBJS += \
./src/algo/8/search_8.o \

OBJS_SSE41 += \
./src/algo/8/8_simd_nw_sse2.o \
./src/algo/8/search_8_util_sse2.o \
./src/algo/8/8_simd_sw_sse2.o

OBJS_AVX2 += \
./src/algo/8/8_simd_nw_avx2.o \
./src/algo/8/8_simd_sw_avx2.o \
./src/algo/8/search_8_util_avx2.o

USER_OBJS += \
./src/algo/8/search_8.h \
./src/algo/8/search_8_util.h

TO_CLEAN +=