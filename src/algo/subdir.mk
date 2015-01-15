OBJS += \
./src/algo/aligner.o \
./src/algo/searcher.o \
./src/algo/manager.o \
./src/algo/smith_waterman_63.o \
./src/algo/needleman_wunsch_63.o \
./src/algo/needleman_wunsch_sellers_63.o \
./src/algo/align.o \
./src/algo/cigar.o \
./src/algo/16_simd_nw.o \
./src/algo/16_simd_sw.o \
./src/algo/8_simd_nw.o

USER_OBJS += \
./src/algo/aligner.h \
./src/algo/searcher.h \
./src/algo/search.h \
./src/algo/manager.h \
./src/algo/align.h \
./src/algo/align_simd.h

TO_CLEAN +=