################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
system/platform.obj: ../system/platform.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="system/platform.pp" --obj_directory="system" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

system/platform_osal.obj: ../system/platform_osal.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="system/platform_osal.pp" --obj_directory="system" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

system/resourcemgr.obj: ../system/resourcemgr.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="system/resourcemgr.pp" --obj_directory="system" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


