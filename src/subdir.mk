OBJS += \
./src/matrices.o \
./src/query.o \
./src/libssa.o \
./src/db_iterator.o

USER_OBJS += \
./src/libssa.h \
./src/libsdb.h \
./src/libssa_datatypes.h \
./src/internal_datatypes.h \
./src/matrices.h \
./src/query.h \
./src/db_iterator.h

TO_CLEAN +=