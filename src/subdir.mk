OBJS += \
./src/util.o \
./src/matrices.o \
./src/query.o \
./src/libssa.o \
./src/util_sequence.o \
./src/db_iterator.o

USER_OBJS += \
./src/libssa.h \
./src/libsdb.h \
./src/libssa_datatypes.h \
./src/internal_datatypes.h \
./src/util.h \
./src/matrices.h \
./src/query.h \
./src/util_sequence.h \
./src/db_iterator.h

TO_CLEAN +=