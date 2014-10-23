################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../tests/algo/test_align.o \
../tests/algo/test_aligner.o \
../tests/algo/test_cigar.o \
../tests/algo/test_manager.o \
../tests/algo/test_searcher.o \
../tests/algo/test_smith_waterman_63.o 

C_SRCS += \
../tests/algo/test_align.c \
../tests/algo/test_aligner.c \
../tests/algo/test_cigar.c \
../tests/algo/test_manager.c \
../tests/algo/test_searcher.c \
../tests/algo/test_smith_waterman_63.c 

OBJS += \
./tests/algo/test_align.o \
./tests/algo/test_aligner.o \
./tests/algo/test_cigar.o \
./tests/algo/test_manager.o \
./tests/algo/test_searcher.o \
./tests/algo/test_smith_waterman_63.o 

C_DEPS += \
./tests/algo/test_align.d \
./tests/algo/test_aligner.d \
./tests/algo/test_cigar.d \
./tests/algo/test_manager.d \
./tests/algo/test_searcher.d \
./tests/algo/test_smith_waterman_63.d 


# Each subdirectory must supply rules for building sources it contributes
tests/algo/%.o: ../tests/algo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


