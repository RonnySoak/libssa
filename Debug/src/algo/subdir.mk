################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/algo/align.o \
../src/algo/aligner.o \
../src/algo/cigar.o \
../src/algo/manager.o \
../src/algo/needleman_wunsch_63.o \
../src/algo/needleman_wunsch_sellers_63.o \
../src/algo/searcher.o \
../src/algo/smith_waterman_63.o 

C_SRCS += \
../src/algo/align.c \
../src/algo/aligner.c \
../src/algo/cigar.c \
../src/algo/manager.c \
../src/algo/needleman_wunsch_63.c \
../src/algo/needleman_wunsch_sellers_63.c \
../src/algo/search16.c \
../src/algo/search16s.c \
../src/algo/search7.c \
../src/algo/searcher.c \
../src/algo/smith_waterman_63.c 

OBJS += \
./src/algo/align.o \
./src/algo/aligner.o \
./src/algo/cigar.o \
./src/algo/manager.o \
./src/algo/needleman_wunsch_63.o \
./src/algo/needleman_wunsch_sellers_63.o \
./src/algo/search16.o \
./src/algo/search16s.o \
./src/algo/search7.o \
./src/algo/searcher.o \
./src/algo/smith_waterman_63.o 

C_DEPS += \
./src/algo/align.d \
./src/algo/aligner.d \
./src/algo/cigar.d \
./src/algo/manager.d \
./src/algo/needleman_wunsch_63.d \
./src/algo/needleman_wunsch_sellers_63.d \
./src/algo/search16.d \
./src/algo/search16s.d \
./src/algo/search7.d \
./src/algo/searcher.d \
./src/algo/smith_waterman_63.d 


# Each subdirectory must supply rules for building sources it contributes
src/algo/%.o: ../src/algo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


