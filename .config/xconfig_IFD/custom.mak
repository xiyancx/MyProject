## THIS IS A GENERATED FILE -- DO NOT EDIT
.configuro: .libraries,e66 linker.cmd \
  package/cfg/IFD_pe66.oe66 \

linker.cmd: package/cfg/IFD_pe66.xdl
	$(SED) 's"^\"\(package/cfg/IFD_pe66cfg.cmd\)\"$""\"C:/work/liu/XGD/IFD/project_20260410_ram/.config/xconfig_IFD/\1\""' package/cfg/IFD_pe66.xdl > $@
