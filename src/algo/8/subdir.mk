OBJS += \
./src/algo/8/search_8.o \
./src/algo/8/8_simd_sse41_nw.o \
./src/algo/8/8_simd_sse41_sw.o \
./src/algo/8/search_8_avx2_util.o \
./src/algo/8/search_8_sse41_util.o

USER_OBJS += \
./src/algo/8/search_8.h

TO_CLEAN +=