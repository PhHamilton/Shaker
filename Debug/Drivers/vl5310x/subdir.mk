################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/vl5310x/vl53l0x_api.c \
../Drivers/vl5310x/vl53l0x_api_calibration.c \
../Drivers/vl5310x/vl53l0x_api_core.c \
../Drivers/vl5310x/vl53l0x_api_ranging.c \
../Drivers/vl5310x/vl53l0x_api_strings.c \
../Drivers/vl5310x/vl53l0x_platform_log.c \
../Drivers/vl5310x/vl53l0x_tof.c 

OBJS += \
./Drivers/vl5310x/vl53l0x_api.o \
./Drivers/vl5310x/vl53l0x_api_calibration.o \
./Drivers/vl5310x/vl53l0x_api_core.o \
./Drivers/vl5310x/vl53l0x_api_ranging.o \
./Drivers/vl5310x/vl53l0x_api_strings.o \
./Drivers/vl5310x/vl53l0x_platform_log.o \
./Drivers/vl5310x/vl53l0x_tof.o 

C_DEPS += \
./Drivers/vl5310x/vl53l0x_api.d \
./Drivers/vl5310x/vl53l0x_api_calibration.d \
./Drivers/vl5310x/vl53l0x_api_core.d \
./Drivers/vl5310x/vl53l0x_api_ranging.d \
./Drivers/vl5310x/vl53l0x_api_strings.d \
./Drivers/vl5310x/vl53l0x_platform_log.d \
./Drivers/vl5310x/vl53l0x_tof.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/vl5310x/%.o Drivers/vl5310x/%.su Drivers/vl5310x/%.cyclo: ../Drivers/vl5310x/%.c Drivers/vl5310x/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G031xx -c -I../Core/Inc -I"/Users/philip/STM32CubeIDE/workspace_1.12.1/Shaker/Drivers/vl5310x" -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-vl5310x

clean-Drivers-2f-vl5310x:
	-$(RM) ./Drivers/vl5310x/vl53l0x_api.cyclo ./Drivers/vl5310x/vl53l0x_api.d ./Drivers/vl5310x/vl53l0x_api.o ./Drivers/vl5310x/vl53l0x_api.su ./Drivers/vl5310x/vl53l0x_api_calibration.cyclo ./Drivers/vl5310x/vl53l0x_api_calibration.d ./Drivers/vl5310x/vl53l0x_api_calibration.o ./Drivers/vl5310x/vl53l0x_api_calibration.su ./Drivers/vl5310x/vl53l0x_api_core.cyclo ./Drivers/vl5310x/vl53l0x_api_core.d ./Drivers/vl5310x/vl53l0x_api_core.o ./Drivers/vl5310x/vl53l0x_api_core.su ./Drivers/vl5310x/vl53l0x_api_ranging.cyclo ./Drivers/vl5310x/vl53l0x_api_ranging.d ./Drivers/vl5310x/vl53l0x_api_ranging.o ./Drivers/vl5310x/vl53l0x_api_ranging.su ./Drivers/vl5310x/vl53l0x_api_strings.cyclo ./Drivers/vl5310x/vl53l0x_api_strings.d ./Drivers/vl5310x/vl53l0x_api_strings.o ./Drivers/vl5310x/vl53l0x_api_strings.su ./Drivers/vl5310x/vl53l0x_platform_log.cyclo ./Drivers/vl5310x/vl53l0x_platform_log.d ./Drivers/vl5310x/vl53l0x_platform_log.o ./Drivers/vl5310x/vl53l0x_platform_log.su ./Drivers/vl5310x/vl53l0x_tof.cyclo ./Drivers/vl5310x/vl53l0x_tof.d ./Drivers/vl5310x/vl53l0x_tof.o ./Drivers/vl5310x/vl53l0x_tof.su

.PHONY: clean-Drivers-2f-vl5310x

