################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bootloader_app_pc.c 

OBJS += \
./src/bootloader_app_pc.o 

C_DEPS += \
./src/bootloader_app_pc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/oem/eclipse-workspace/bootloader_app_pc/FILE_HANDLE" -I"/home/oem/eclipse-workspace/bootloader_app_pc/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


