
# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/avltree.c \
../src/common.c \
../src/cpu_stats.c \
../src/database.c \
../src/display.c \
../src/energy.c \
../src/eperf.c \
../src/m_stats.c \
../src/pid_stats.c \
../src/ptop.c 

OBJS += \
./src/avltree.o \
./src/common.o \
./src/cpu_stats.o \
./src/database.o \
./src/display.o \
./src/energy.o \
./src/eperf.o \
./src/m_stats.o \
./src/pid_stats.o \
./src/ptop.o 

C_DEPS += \
./src/avltree.d \
./src/common.d \
./src/cpu_stats.d \
./src/database.d \
./src/display.d \
./src/energy.d \
./src/eperf.d \
./src/m_stats.d \
./src/pid_stats.d \
./src/ptop.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '