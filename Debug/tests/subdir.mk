################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../tests/check_libssa.o \
../tests/test_db_iterator.o \
../tests/test_matrices.o \
../tests/test_minheap.o \
../tests/test_query.o \
../tests/test_util.o \
../tests/test_util_sequence.o 

C_SRCS += \
../tests/check_libssa.c \
../tests/test_db_iterator.c \
../tests/test_matrices.c \
../tests/test_minheap.c \
../tests/test_query.c \
../tests/test_util.c \
../tests/test_util_sequence.c 

OBJS += \
./tests/check_libssa.o \
./tests/test_db_iterator.o \
./tests/test_matrices.o \
./tests/test_minheap.o \
./tests/test_query.o \
./tests/test_util.o \
./tests/test_util_sequence.o 

C_DEPS += \
./tests/check_libssa.d \
./tests/test_db_iterator.d \
./tests/test_matrices.d \
./tests/test_minheap.d \
./tests/test_query.d \
./tests/test_util.d \
./tests/test_util_sequence.d 


# Each subdirectory must supply rules for building sources it contributes
tests/%.o: ../tests/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


