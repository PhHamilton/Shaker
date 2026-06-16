################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/VL53L0x/vl53l0x.c 

OBJS += \
./Drivers/VL53L0x/vl53l0x.o 

C_DEPS += \
./Drivers/VL53L0x/vl53l0x.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/VL53L0x/%.o Drivers/VL53L0x/%.su Drivers/VL53L0x/%.cyclo: ../Drivers/VL53L0x/%.c Drivers/VL53L0x/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32G031xx -c -I../Core/Inc -I"/Users/philip/STM32CubeIDE/workspace_1.12.1/Shaker/Drivers/VL53L0x" -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-VL53L0x

clean-Drivers-2f-VL53L0x:
	-$(RM) ./Drivers/VL53L0x/vl53l0x.cyclo ./Drivers/VL53L0x/vl53l0x.d ./Drivers/VL53L0x/vl53l0x.o ./Drivers/VL53L0x/vl53l0x.su

.PHONY: clean-Drivers-2f-VL53L0x

