################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/db_iterator.o \
../src/libssa.o \
../src/matrices.o \
../src/query.o \
../src/util.o \
../src/util_sequence.o 

C_SRCS += \
../src/db_iterator.c \
../src/libssa.c \
../src/libssa_example.c \
../src/matrices.c \
../src/query.c \
../src/util.c \
../src/util_sequence.c 

OBJS += \
./src/db_iterator.o \
./src/libssa.o \
./src/libssa_example.o \
./src/matrices.o \
./src/query.o \
./src/util.o \
./src/util_sequence.o 

C_DEPS += \
./src/db_iterator.d \
./src/libssa.d \
./src/libssa_example.d \
./src/matrices.d \
./src/query.d \
./src/util.d \
./src/util_sequence.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


