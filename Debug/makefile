################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./driver/nimu_eth_c667x.obj" \
"./driver/c66x_uart.obj" \
"./driver/c66x_spi.obj" \
"./driver/c66x_nor.obj" \
"./driver/c66x_nand.obj" \
"./driver/c66x_i2c.obj" \
"./driver/c66x_gpio.obj" \
"./driver/c66x_eeprom.obj" \
"./system/resourcemgr.obj" \
"./system/platform_osal.obj" \
"./system/platform.obj" \
"./srio.obj" \
"./protocol.obj" \
"./net_init.obj" \
"./nand_op.obj" \
"./main.obj" \
"./collect_result_task.obj" \
"./adaptive_filter.obj" \
-l"ti.csl.ae66" \
-l"dsplib.ae66" \
-l"mathlib.ae66" \
-l"libc.a" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include system/subdir_vars.mk
-include driver/subdir_vars.mk
-include subdir_rules.mk
-include system/subdir_rules.mk
-include driver/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: IFD_0410_RAM.out

# Tool invocations
IFD_0410_RAM.out: $(OBJS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: C6000 Linker'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far -z -m"IFD_0410_RAM.map" -i"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/lib" -i"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" -i"C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl/lib" -i"C:/Program Files/ti/dsplib_c66x_3_1_0_0/lib" -i"C:/Program Files/ti/mathlib_c66x_3_0_1_1/lib" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="IFD_0410_RAM_linkInfo.xml" --rom_model -o "IFD_0410_RAM.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(C6000_EXECUTABLE_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(GEN_OPTS__QUOTED)$(GEN_CMDS__QUOTED) "IFD_0410_RAM.out"
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-$(RM) "adaptive_filter.pp" "collect_result_task.pp" "main.pp" "nand_op.pp" "net_init.pp" "protocol.pp" "srio.pp" "system\platform.pp" "system\platform_osal.pp" "system\resourcemgr.pp" "driver\c66x_eeprom.pp" "driver\c66x_gpio.pp" "driver\c66x_i2c.pp" "driver\c66x_nand.pp" "driver\c66x_nor.pp" "driver\c66x_spi.pp" "driver\c66x_uart.pp" "driver\nimu_eth_c667x.pp" 
	-$(RM) "adaptive_filter.obj" "collect_result_task.obj" "main.obj" "nand_op.obj" "net_init.obj" "protocol.obj" "srio.obj" "system\platform.obj" "system\platform_osal.obj" "system\resourcemgr.obj" "driver\c66x_eeprom.obj" "driver\c66x_gpio.obj" "driver\c66x_i2c.obj" "driver\c66x_nand.obj" "driver\c66x_nor.obj" "driver\c66x_spi.obj" "driver\c66x_uart.obj" "driver\nimu_eth_c667x.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

