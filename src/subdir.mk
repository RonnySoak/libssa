OBJS += \
./src/matrices.o \
./src/query.o \
./src/libssa.o \
./src/db_adapter.o \
./src/cpu_config.o

USER_OBJS += \
./src/libssa.h \
./src/libsdb.h \
./src/libssa_datatypes.h \
./src/internal_datatypes.h \
./src/matrices.h \
./src/query.h \
./src/db_adapter.h \
./src/cpu_config.h

TO_CLEAN +=