############################################################
#     Copyright (c) 2003-2012, Broadcom Corporation
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
# $brcm_Log: $
# 
############################################################

ifeq (${VERBOSE}, )
Q_ := @
else
Q_ :=
endif

LINUX ?= /opt/brcm/linux
LINUX_VER_GE_2_6 ?= $(shell (grep 'PATCHLEVEL = 6' ${LINUX}/Makefile >/dev/null && echo 'y'))
LINUX_VER_GE_3_3 ?= $(shell (grep 'PATCHLEVEL = 3' ${LINUX}/Makefile >/dev/null && echo 'y'))
ifeq ($(LINUX_VER_GE_2_6),y)
LINUX_VER_EQ_2_6_31 := $(shell (grep 'SUBLEVEL = 31' ${LINUX}/Makefile >/dev/null && echo 'y'))
endif
ifeq ($(LINUX_VER_GE_2_6),y)
LINUX_VER_EQ_2_6_37 := $(shell (grep 'SUBLEVEL = 37' ${LINUX}/Makefile >/dev/null && echo 'y'))
endif


ifeq ($(LINUX_VER_EQ_2_6_31),y)
include ${TOP_DIR}/Common.2.6.31.make
endif

ifeq ($(LINUX_VER_EQ_2_6_37),y)
include ${TOP_DIR}/Common.2.6.31.make
endif

ifeq ($(LINUX_VER_GE_3_3),y)
LINUX_VER_EQ_2_6_37 :=y
include ${TOP_DIR}/Common.2.6.31.make
endif

# set defaults
B_REFSW_DEBUG ?= y

# You must have BSEAV, MAGNUM, already defined before including this makefile
include ${BSEAV}/build/refsw_inc.mak

include ${TOP_DIR}/chip.mak

BCM_BOARD = bcm${BCM_BOARD_NO}


ifeq (${BCHP_VER},)
BCHP_VER:=B0
endif

CFLAGS += -DBCHP_CHIP=${BCHP_CHIP} -DBCHP_VER=BCHP_VER_${BCHP_VER}

# Include required modules for IP Streamer below that are not in refsw_inc.mak
include $(MAGNUM)/basemodules/chp/build/bchp.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/basemodules/err/berr.inc
include $(MAGNUM)/basemodules/int/bint.inc
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/mem/bmem.inc
include $(MAGNUM)/basemodules/reg/breg.inc
include $(MAGNUM)/basemodules/std/bstd.inc
include $(MAGNUM)/basemodules/tmr/btmr.inc
include $(MAGNUM)/commonutils/lst/blst.inc

#ifeq ($(NETACCEL_SUPPORT),y)
#include $(MAGNUM)/portinginterface/dma/bdma.inc
#endif

# Create single list of included paths
MAGNUM_INCLUDES := $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
# Create single list of included definitions
MAGNUM_DEFINES := $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))
ifeq ($(USE_DMA_PI), y)
# Create a list of included sources
MAGNUM_SOURCES := $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES))
else
MAGNUM_SOURCES :=
endif

# Convert includes and defines into CFLAGS
CFLAGS += $(addprefix -I,$(MAGNUM_INCLUDES))
CFLAGS += $(addprefix -D,$(MAGNUM_DEFINES))


#section for Common.make#
# Configure linux kernel source
# Output is LINUX_INC
#
# defines TOOLCHAIN_DIR, B_REFSW_DEBUG, B_REFSW_ARCH
##############################################

ifeq ($(B_REFSW_DEBUG),y)
BCM_OBJ_DIR=$(TOP_DIR)/$(B_REFSW_ARCH).debug
else
BCM_OBJ_DIR=$(TOP_DIR)/$(B_REFSW_ARCH).release
endif

$(BCM_OBJ_DIR):
	${Q_}mkdir -p $(BCM_OBJ_DIR)
	
.PHONY: checkdirs
checkdirs: $(LINUX_INC) $(STD_INC) $(GCC_BASE) $(BSEAV) $(BCM_OBJ_DIR)



ifeq ($(B_REFSW_DEBUG),y)
CFLAGS += -DCONFIG_PROC_FS
endif

#########################################################
#
# soft float
#
# PR8054: We can't use the fpu in kernel mode unless it's
# in a critical section. The Linux exception handler doesn't
# store the fpu registers.
ifeq ($(OS),linuxkernel)
CFLAGS += -msoft-float
FLOATLIB_OBJS += fp-bit-single.o fp-bit-double.o
FLOATLIB_DIR = $(BSEAV)/linux/driver/floatlib/src
endif

#########################################################
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
# Clean up drivers: use clean_drv in your Makefile's clean
#
clean_drv: $(OTHER_CLEANS)
	${Q_}$(RM) -r $(BCM_OBJ_DIR)


