################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/util/minheap.o \
../src/util/thread_pool.o 

C_SRCS += \
../src/util/minheap.c \
../src/util/thread_pool.c 

OBJS += \
./src/util/minheap.o \
./src/util/thread_pool.o 

C_DEPS += \
./src/util/minheap.d \
./src/util/thread_pool.d 


# Each subdirectory must supply rules for building sources it contributes
src/util/%.o: ../src/util/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


