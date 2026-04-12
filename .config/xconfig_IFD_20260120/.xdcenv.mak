#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/PROGRA~1/ti/ndk_2_21_02_43/packages;C:/PROGRA~1/ti/ipc_1_24_03_32/packages;C:/PROGRA~1/ti/omp_1_01_03_02/packages;C:/PROGRA~1/ti/bios_6_33_06_50/packages;C:/PROGRA~1/ti/pdk_C6678_1_1_2_6/packages;C:/PROGRA~1/ti/ccsv5/ccs_base;C:/work/liu/XGD/IFD/project_20260120/.config
override XDCROOT = C:/PROGRA~1/ti/xdctools_3_23_04_60
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/PROGRA~1/ti/ndk_2_21_02_43/packages;C:/PROGRA~1/ti/ipc_1_24_03_32/packages;C:/PROGRA~1/ti/omp_1_01_03_02/packages;C:/PROGRA~1/ti/bios_6_33_06_50/packages;C:/PROGRA~1/ti/pdk_C6678_1_1_2_6/packages;C:/PROGRA~1/ti/ccsv5/ccs_base;C:/work/liu/XGD/IFD/project_20260120/.config;C:/PROGRA~1/ti/xdctools_3_23_04_60/packages;..
HOSTOS = Windows
endif
