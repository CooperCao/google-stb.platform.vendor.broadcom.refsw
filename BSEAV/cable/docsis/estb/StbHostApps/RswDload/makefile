rps7125: estbswdload.inc rProgramStoreCustomerApi.c rProgramStoreCustomerApi.h rProgramStoreHost.c
ifeq ($(ARM_PLATFORM), yes)
	arm-linux-uclibcgnueabi-gcc rProgramStoreHost.c rProgramStoreCustomerApi.c ../../cm_bsp_v2/bsp_common/common_src/crc.c  -DPROCESSOR_TYPE=BCMMIPSEL -DSTANDALONE_APP -I../../cm_bsp_v2/bsp_common/common_inc -I../../Stb/StbDocsisApi/Dsg -o rps7125
else
	mipsel-linux-uclibc-gcc rProgramStoreHost.c rProgramStoreCustomerApi.c ../../cm_bsp_v2/bsp_common/common_src/crc.c  -DPROCESSOR_TYPE=BCMMIPSEL -DSTANDALONE_APP -I../../cm_bsp_v2/bsp_common/common_inc -I../../Stb/StbDocsisApi/Dsg -o rps7125
endif