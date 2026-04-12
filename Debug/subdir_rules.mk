################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
configPkg/linker.cmd: ../IFD.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"C:/Program Files/ti/xdctools_3_23_04_60/xs" --xdcpath="C:/Program Files/ti/ndk_2_21_02_43/packages;C:/Program Files/ti/ipc_1_24_03_32/packages;C:/Program Files/ti/omp_1_01_03_02/packages;C:/Program Files/ti/bios_6_33_06_50/packages;C:/Program Files/ti/pdk_C6678_1_1_2_6/packages;C:/Program Files/ti/ccsv5/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.elf.C66 -p ti.platforms.evm6678 -r debug -c "C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4" "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/compiler.opt: | configPkg/linker.cmd
configPkg/: | configPkg/linker.cmd

adaptive_filter.obj: ../adaptive_filter.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="adaptive_filter.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

collect_result_task.obj: ../collect_result_task.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="collect_result_task.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

nand_op.obj: ../nand_op.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="nand_op.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

net_init.obj: ../net_init.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="net_init.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

protocol.obj: ../protocol.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="protocol.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

srio.obj: ../srio.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="C:/Program Files/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/" --include_path="C:/Program Files/ti/dsplib_c66x_3_1_0_0/packages/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/" --include_path="C:/Program Files/ti/mathlib_c66x_3_0_1_1/packages/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/transport/ndk/nimu/" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --include_path="C:/Program Files/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="C:/work/liu/XGD/IFD/project_20260410_ram" --define=TL6678ZH_EVM --define=C66_PLATFORMS --define=_INCLUDE_NIMU_CODE --display_error_number --diag_warning=225 --diag_wrap=off --mem_model:data=far --preproc_with_compile --preproc_dependency="srio.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


