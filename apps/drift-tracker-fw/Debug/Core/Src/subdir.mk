################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/lr_fhss_mac.c \
../Core/Src/main.c \
../Core/Src/radio.c \
../Core/Src/radio_board_if.c \
../Core/Src/radio_driver.c \
../Core/Src/radio_fw.c \
../Core/Src/stm32_adv_trace.c \
../Core/Src/stm32_adv_trace_if_template.c \
../Core/Src/stm32_lpm.c \
../Core/Src/stm32_mem.c \
../Core/Src/stm32_seq.c \
../Core/Src/stm32_systime.c \
../Core/Src/stm32_systime_if.c \
../Core/Src/stm32_timer.c \
../Core/Src/stm32_timer_if.c \
../Core/Src/stm32_tiny_sscanf.c \
../Core/Src/stm32_tiny_vsnprintf.c \
../Core/Src/stm32wlxx_hal_msp.c \
../Core/Src/stm32wlxx_it.c \
../Core/Src/subghz.c \
../Core/Src/sys_debug.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32wlxx.c \
../Core/Src/wl_lr_fhss.c 

OBJS += \
./Core/Src/lr_fhss_mac.o \
./Core/Src/main.o \
./Core/Src/radio.o \
./Core/Src/radio_board_if.o \
./Core/Src/radio_driver.o \
./Core/Src/radio_fw.o \
./Core/Src/stm32_adv_trace.o \
./Core/Src/stm32_adv_trace_if_template.o \
./Core/Src/stm32_lpm.o \
./Core/Src/stm32_mem.o \
./Core/Src/stm32_seq.o \
./Core/Src/stm32_systime.o \
./Core/Src/stm32_systime_if.o \
./Core/Src/stm32_timer.o \
./Core/Src/stm32_timer_if.o \
./Core/Src/stm32_tiny_sscanf.o \
./Core/Src/stm32_tiny_vsnprintf.o \
./Core/Src/stm32wlxx_hal_msp.o \
./Core/Src/stm32wlxx_it.o \
./Core/Src/subghz.o \
./Core/Src/sys_debug.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32wlxx.o \
./Core/Src/wl_lr_fhss.o 

C_DEPS += \
./Core/Src/lr_fhss_mac.d \
./Core/Src/main.d \
./Core/Src/radio.d \
./Core/Src/radio_board_if.d \
./Core/Src/radio_driver.d \
./Core/Src/radio_fw.d \
./Core/Src/stm32_adv_trace.d \
./Core/Src/stm32_adv_trace_if_template.d \
./Core/Src/stm32_lpm.d \
./Core/Src/stm32_mem.d \
./Core/Src/stm32_seq.d \
./Core/Src/stm32_systime.d \
./Core/Src/stm32_systime_if.d \
./Core/Src/stm32_timer.d \
./Core/Src/stm32_timer_if.d \
./Core/Src/stm32_tiny_sscanf.d \
./Core/Src/stm32_tiny_vsnprintf.d \
./Core/Src/stm32wlxx_hal_msp.d \
./Core/Src/stm32wlxx_it.d \
./Core/Src/subghz.d \
./Core/Src/sys_debug.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32wlxx.d \
./Core/Src/wl_lr_fhss.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WL5Mxx -c -I../Core/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/lr_fhss_mac.cyclo ./Core/Src/lr_fhss_mac.d ./Core/Src/lr_fhss_mac.o ./Core/Src/lr_fhss_mac.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/radio.cyclo ./Core/Src/radio.d ./Core/Src/radio.o ./Core/Src/radio.su ./Core/Src/radio_board_if.cyclo ./Core/Src/radio_board_if.d ./Core/Src/radio_board_if.o ./Core/Src/radio_board_if.su ./Core/Src/radio_driver.cyclo ./Core/Src/radio_driver.d ./Core/Src/radio_driver.o ./Core/Src/radio_driver.su ./Core/Src/radio_fw.cyclo ./Core/Src/radio_fw.d ./Core/Src/radio_fw.o ./Core/Src/radio_fw.su ./Core/Src/stm32_adv_trace.cyclo ./Core/Src/stm32_adv_trace.d ./Core/Src/stm32_adv_trace.o ./Core/Src/stm32_adv_trace.su ./Core/Src/stm32_adv_trace_if_template.cyclo ./Core/Src/stm32_adv_trace_if_template.d ./Core/Src/stm32_adv_trace_if_template.o ./Core/Src/stm32_adv_trace_if_template.su ./Core/Src/stm32_lpm.cyclo ./Core/Src/stm32_lpm.d ./Core/Src/stm32_lpm.o ./Core/Src/stm32_lpm.su ./Core/Src/stm32_mem.cyclo ./Core/Src/stm32_mem.d ./Core/Src/stm32_mem.o ./Core/Src/stm32_mem.su ./Core/Src/stm32_seq.cyclo ./Core/Src/stm32_seq.d ./Core/Src/stm32_seq.o ./Core/Src/stm32_seq.su ./Core/Src/stm32_systime.cyclo ./Core/Src/stm32_systime.d ./Core/Src/stm32_systime.o ./Core/Src/stm32_systime.su ./Core/Src/stm32_systime_if.cyclo ./Core/Src/stm32_systime_if.d ./Core/Src/stm32_systime_if.o ./Core/Src/stm32_systime_if.su ./Core/Src/stm32_timer.cyclo ./Core/Src/stm32_timer.d ./Core/Src/stm32_timer.o ./Core/Src/stm32_timer.su ./Core/Src/stm32_timer_if.cyclo ./Core/Src/stm32_timer_if.d ./Core/Src/stm32_timer_if.o ./Core/Src/stm32_timer_if.su ./Core/Src/stm32_tiny_sscanf.cyclo ./Core/Src/stm32_tiny_sscanf.d ./Core/Src/stm32_tiny_sscanf.o ./Core/Src/stm32_tiny_sscanf.su ./Core/Src/stm32_tiny_vsnprintf.cyclo ./Core/Src/stm32_tiny_vsnprintf.d ./Core/Src/stm32_tiny_vsnprintf.o ./Core/Src/stm32_tiny_vsnprintf.su ./Core/Src/stm32wlxx_hal_msp.cyclo ./Core/Src/stm32wlxx_hal_msp.d ./Core/Src/stm32wlxx_hal_msp.o ./Core/Src/stm32wlxx_hal_msp.su ./Core/Src/stm32wlxx_it.cyclo ./Core/Src/stm32wlxx_it.d ./Core/Src/stm32wlxx_it.o ./Core/Src/stm32wlxx_it.su ./Core/Src/subghz.cyclo ./Core/Src/subghz.d ./Core/Src/subghz.o ./Core/Src/subghz.su ./Core/Src/sys_debug.cyclo ./Core/Src/sys_debug.d ./Core/Src/sys_debug.o ./Core/Src/sys_debug.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32wlxx.cyclo ./Core/Src/system_stm32wlxx.d ./Core/Src/system_stm32wlxx.o ./Core/Src/system_stm32wlxx.su ./Core/Src/wl_lr_fhss.cyclo ./Core/Src/wl_lr_fhss.d ./Core/Src/wl_lr_fhss.o ./Core/Src/wl_lr_fhss.su

.PHONY: clean-Core-2f-Src

