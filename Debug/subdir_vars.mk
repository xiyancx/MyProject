################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CFG_SRCS += \
../IFD.cfg 

C_SRCS += \
../adaptive_filter.c \
../collect_result_task.c \
../main.c \
../nand_op.c \
../net_init.c \
../protocol.c \
../srio.c 

OBJS += \
./adaptive_filter.obj \
./collect_result_task.obj \
./main.obj \
./nand_op.obj \
./net_init.obj \
./protocol.obj \
./srio.obj 

C_DEPS += \
./adaptive_filter.pp \
./collect_result_task.pp \
./main.pp \
./nand_op.pp \
./net_init.pp \
./protocol.pp \
./srio.pp 

GEN_MISC_DIRS += \
./configPkg/ 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_OPTS += \
./configPkg/compiler.opt 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_FILES__QUOTED += \
"configPkg\linker.cmd" \
"configPkg\compiler.opt" 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

C_DEPS__QUOTED += \
"adaptive_filter.pp" \
"collect_result_task.pp" \
"main.pp" \
"nand_op.pp" \
"net_init.pp" \
"protocol.pp" \
"srio.pp" 

OBJS__QUOTED += \
"adaptive_filter.obj" \
"collect_result_task.obj" \
"main.obj" \
"nand_op.obj" \
"net_init.obj" \
"protocol.obj" \
"srio.obj" 

GEN_CMDS__FLAG += \
-l"./configPkg/linker.cmd" 

GEN_OPTS__FLAG += \
--cmd_file="./configPkg/compiler.opt" 

C_SRCS__QUOTED += \
"../adaptive_filter.c" \
"../collect_result_task.c" \
"../main.c" \
"../nand_op.c" \
"../net_init.c" \
"../protocol.c" \
"../srio.c" 


