################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
driver/c66x_eeprom.obj: ../driver/c66x_eeprom.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="driver/c66x_eeprom.pp" --obj_directory="driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

driver/c66x_gpio.obj: ../driver/c66x_gpio.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="driver/c66x_gpio.pp" --obj_directory="driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

driver/c66x_i2c.obj: ../driver/c66x_i2c.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="driver/c66x_i2c.pp" --obj_directory="driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

driver/c66x_nand.obj: ../driver/c66x_nand.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="driver/c66x_nand.pp" --obj_directory="driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

driver/c66x_nor.obj: ../driver/c66x_nor.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="driver/c66x_nor.pp" --obj_directory="driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

driver/c66x_spi.obj: ../driver/c66x_spi.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="driver/c66x_spi.pp" --obj_directory="driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

driver/c66x_uart.obj: ../driver/c66x_uart.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="driver/c66x_uart.pp" --obj_directory="driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

driver/nimu_eth_c667x.obj: ../driver/nimu_eth_c667x.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="driver/nimu_eth_c667x.pp" --obj_directory="driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


