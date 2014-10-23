################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../to_evaluate/main.c \
../to_evaluate/swalign.c \
../to_evaluate/swipe_chunks.c \
../to_evaluate/swipe_mpi.c \
../to_evaluate/swipe_revised.c \
../to_evaluate/swipe_util.c 

OBJS += \
./to_evaluate/main.o \
./to_evaluate/swalign.o \
./to_evaluate/swipe_chunks.o \
./to_evaluate/swipe_mpi.o \
./to_evaluate/swipe_revised.o \
./to_evaluate/swipe_util.o 

C_DEPS += \
./to_evaluate/main.d \
./to_evaluate/swalign.d \
./to_evaluate/swipe_chunks.d \
./to_evaluate/swipe_mpi.d \
./to_evaluate/swipe_revised.d \
./to_evaluate/swipe_util.d 


# Each subdirectory must supply rules for building sources it contributes
to_evaluate/%.o: ../to_evaluate/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


