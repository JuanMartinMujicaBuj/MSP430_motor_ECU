################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/Users/marco/OneDrive/Documents/ITBA/Micros y Control/CCStudio/ccs/tools/compiler/ti-cgt-msp430_18.12.1.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./adc_2.obj" \
"./bujia.obj" \
"./comms.obj" \
"./dc_motor.obj" \
"./gpio.obj" \
"./hw_timer.obj" \
"./inputCapture.obj" \
"./joystick_2.obj" \
"./main.obj" \
"./mcp2515.obj" \
"./optoacoplador.obj" \
"./pwm.obj" \
"./rti.obj" \
"./servo.obj" \
"./spi.obj" \
"./system.obj" \
"./uart.obj" \
"../lnk_msp430g2553.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
tablero_motor_2_12.out \

EXE_OUTPUTS__QUOTED += \
"tablero_motor_2_12.out" \

BIN_OUTPUTS += \
tablero_motor_2_12.hex \

BIN_OUTPUTS__QUOTED += \
"tablero_motor_2_12.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "tablero_motor_2_12.out"

# Tool invocations
tablero_motor_2_12.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: MSP430 Linker'
	"C:/Users/marco/OneDrive/Documents/ITBA/Micros y Control/CCStudio/ccs/tools/compiler/ti-cgt-msp430_18.12.1.LTS/bin/cl430" -vmsp --use_hw_mpy=none --advice:power=all --define=__MSP430G2553__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number -z -m"tablero_motor_2_12.map" --heap_size=80 --stack_size=80 -i"C:/Users/marco/OneDrive/Documents/ITBA/Micros y Control/CCStudio/ccs/ccs_base/msp430/include" -i"C:/Users/marco/OneDrive/Documents/ITBA/Micros y Control/CCStudio/ccs/tools/compiler/ti-cgt-msp430_18.12.1.LTS/lib" -i"C:/Users/marco/OneDrive/Documents/ITBA/Micros y Control/CCStudio/ccs/tools/compiler/ti-cgt-msp430_18.12.1.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="tablero_motor_2_12_linkInfo.xml" --use_hw_mpy=none --rom_model -o "tablero_motor_2_12.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

tablero_motor_2_12.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: MSP430 Hex Utility'
	"C:/Users/marco/OneDrive/Documents/ITBA/Micros y Control/CCStudio/ccs/tools/compiler/ti-cgt-msp430_18.12.1.LTS/bin/hex430" --memwidth=8 --romwidth=8 -o "tablero_motor_2_12.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "adc_2.obj" "bujia.obj" "comms.obj" "dc_motor.obj" "gpio.obj" "hw_timer.obj" "inputCapture.obj" "joystick_2.obj" "main.obj" "mcp2515.obj" "optoacoplador.obj" "pwm.obj" "rti.obj" "servo.obj" "spi.obj" "system.obj" "uart.obj" 
	-$(RM) "adc_2.d" "bujia.d" "comms.d" "dc_motor.d" "gpio.d" "hw_timer.d" "inputCapture.d" "joystick_2.d" "main.d" "mcp2515.d" "optoacoplador.d" "pwm.d" "rti.d" "servo.d" "spi.d" "system.d" "uart.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

