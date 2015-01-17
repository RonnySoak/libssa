OBJS += \
./src/algo/aligner.o \
./src/algo/searcher.o \
./src/algo/manager.o \
./src/algo/align.o \
./src/algo/cigar.o

USER_OBJS += \
./src/algo/aligner.h \
./src/algo/searcher.h \
./src/algo/search.h \
./src/algo/manager.h \
./src/algo/align.h \
./src/algo/align_simd.h

TO_CLEAN +=