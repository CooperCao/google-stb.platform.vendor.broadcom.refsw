############################################################
#     Copyright (c) 2003-2011, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 02/09/2001 by Marcus Kellerman
#
# $brcm_Log: $
# 
############################################################

#
# Common driver build for all platforms
# This should not be used to build usermode code
#
# Inputs: SETTOP, BSEAV, BUILDDIR, BCM_CHIP, DRIVERS
# Outputs: Lots of stuff
#

.PHONY: checkdirs

DEBUG ?= y
BCM_ENDIAN ?= le
TRANSMSG_SUPPORT ?= y
#
# Configure linux kernel source
# Output is LINUX_INC
#
include ../common/Linux.make
include ../common/tools.mak

ifeq ($(DEBUG),y)
BCM_OBJ_DIR=$(B_REFSW_ARCH).debug
else
BCM_OBJ_DIR=$(B_REFSW_ARCH).release
endif
LDFLAGS += -G 0

#
# Standard toolchain directories
# See tools.mak for TOOLCHAIN_DIR
#
STD_INC ?= $(TOOLCHAIN_DIR)/mipsel-linux/include
GCC_BASE ?= $(shell (test -d $(TOOLCHAIN_DIR)/lib/gcc-lib && echo $(TOOLCHAIN_DIR)/lib/gcc-lib/mipsel-linux) || echo $(TOOLCHAIN_DIR)/lib/gcc/mipsel-linux)

#
# BEWARE: If you have more than one version of gcc installed, 
# the following line is going to have a problem.
#
GCC_VER = $(shell (ls $(GCC_BASE)))
GCC_INC = $(GCC_BASE)/$(GCC_VER)/include

$(STD_INC):
	@test -d $@ || (echo "STD_INC is incorrect"; test 0 == 1)

$(GCC_BASE):
	@test -d $@ || (echo "GCC_BASE is incorrect"; test 0 == 1)
	
$(SETTOP):
	@test -d $@ || (echo "SETTOP is incorrect"; test 0 == 1)

$(BSEAV):
	@test -d $@ || (echo "BSEAV is incorrect"; test 0 == 1)
	
$(BUILDDIR):
	@test -d $@ || (echo "BUILDDIR is incorrect"; test 0 == 1)
	
$(BCM_OBJ_DIR):
	${Q_}mkdir -p $(BCM_OBJ_DIR)
	
checkdirs: $(LINUX_INC) $(STD_INC) $(GCC_BASE) $(SETTOP) $(BSEAV) $(BUILDDIR) $(BCM_OBJ_DIR)

CFLAGS += \
	-DMPEGV_VIRTUAL_VSYNC \
	-DBCM_KERNEL_CACHE_FLUSH \
	-DBCM_TRICK_MODE_SUPPORT \
	-DUSE_LEGACY_KERNAL_INTERFACE \
	-DUSE_BCM_DEVICE_ARRAY

ifeq ($(LINUX_VER_GE_2_6),y)
# 2.6 flags
CFLAGS += \
	-D__KERNEL__  -Wall -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing \
	-fno-common -ffreestanding -O2 -fomit-frame-pointer -G 0  -mno-abicalls -fno-pic -pipe  \
	-finline-limit=100000  -mabi=32 -march=mips32 -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 \
	-Wa,--trap -DMODULE -mlong-calls -nostdinc
else
# 2.4 flags
CFLAGS += \
	-mips2 -Wa,--trap -nostdinc \
	-fomit-frame-pointer -fno-strict-aliasing -fno-pic \
	-mlong-calls -mno-abicalls \
	-G 0 \
	-Wall
endif

COMMON_DRIVER_INFRASTRUCTURE += \
	bcmdevice.c			\
	bcmkernel.c			\
	bcmmemmgr.c			\
	bcmhw.c				\
	brcm_dbg.c			\
	genericlist.c		\
	intmgr.c			\
	bcmprocfs.c			\
	bcm_linux.c 		\
	bcmcfg.c 			\
	bcminitstack.c		\
	bcmreg.c				\
	bcmio_drv.c

COMMON_DRIVER_BACKEND += \
	bcmplaydrv.c 		\
	bcmdecodedrv.c		\
	bcmwindowdrv.c		\
	bcmdbgdrv.c			\
	bge_intmgr.c		\
	bcmtimer.c			\
	bcmoutputdrv.c		\
	bcmvolumedrv.c		\
	bcmdmadrv.c			

ifeq ($(TRANSMSG_SUPPORT),y)
COMMON_DRIVER_BACKEND += \
    bcmmsgdrv.c         \
    msglib.c			\
	tspsi_validate.c
else
CFLAGS += -DNO_TRANSMSG
endif

# These should be removed. We eventually want all warnings.
CFLAGS += -Wno-comment -Wno-unused -Wno-format

LDFLAGS += --warn-common

# Keep this until the old RFM code goes away
CFLAGS += -DRFM_SIMPLIFIED
# __BCM93725B__ is necessary for any set-top (non PCI), I think this
# is only used in SetTop/kernelinterface/bcmkernel.c/h
CFLAGS += -D__BCM93725B__

ifneq ("$(LINUX_INC)","" )
	CFLAGS += -I$(LINUX_INC)
endif
ifneq ("$(STD_INC)","" )
	CFLAGS += -I$(STD_INC)
endif
ifneq ("$(GCC_INC)","" )
	CFLAGS += -I$(GCC_INC)
endif

ifeq ($(DEBUG),y)
	CFLAGS += -DBRCM_DBG -DBRCM_DEBUG -DCONFIG_PROC_FS -DMEM_DEBUG
endif
ifeq ($(BCM_ENDIAN),le)
	CFLAGS += -D_LITTLE_ENDIAN_ -DBRCM_ENDIAN_LITTLE
else
	CFLAGS += -DBRCM_ENDIAN_BIG
endif
CFLAGS += -DLINUX -O2 -D__KERNEL__ -DMODULE 
ifeq ($(BCM_CHIP),venom2)
CFLAGS += -DBCM7030
else
CFLAGS += -DBCM$(BCM_CHIP)
endif

# additional defines
ifeq ($(findstring $(BCM_CHIP),7312 7313 7314 7315 7317 7318 7319 7320 7327 7328 7329), $(BCM_CHIP))
CFLAGS += -DBCM73XX
endif
ifeq ($(findstring $(BCM_CHIP),7319 7320 7327 7328 7329), $(BCM_CHIP))
CFLAGS += -DBCM732X
endif

# PR8054: We can't use the fpu in kernel mode unless it's
# in a critical section. The Linux exception handler doesn't
# store the fpu registers.
CFLAGS += -msoft-float

# deprecated. drivers should switch to compiling from source
FLOATLIB = $(BSEAV)/linux/driver/floatlib/$(B_REFSW_ARCH)/soft-fp.a

# compiile from source
FLOATLIB_OBJS += fp-bit-single.o fp-bit-double.o
FLOATLIB_DIR = $(BSEAV)/linux/driver/floatlib/src

#
# Driver installation
#
ifeq ($(INSTALL_DIR),)
install:
	$(error INSTALL_DIR is undefined)
else
install:
	@echo "[Install... $(sort $(notdir $(DRIVERS)))]"
	${Q_}$(CP) $(DRIVERS) $(INSTALL_DIR)
endif

#
# Clean up drivers
#
clean: $(OTHER_CLEANS)
	${Q_}$(RM) -r $(BCM_OBJ_DIR)

