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
# Created: 02/09/2001 by Marcus Kellerman
#
# $brcm_Log: $
# 
############################################################

LINUX ?= /opt/brcm/linux

LINUX_PATCHLEVEL = $(shell grep 'PATCHLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_SUBLEVEL = $(shell grep 'SUBLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_VER_GE_2_6_31 = $(shell test $(LINUX_PATCHLEVEL) -eq 6 -a $(LINUX_SUBLEVEL) -ge 31 && echo y)

# linux 2.6.31 and greater uses kbuild for bcmdriver.ko
ifeq ($(LINUX_VER_GE_2_6_31),y)

ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
endif
ifdef DEBUG
B_REFSW_DEBUG ?= $(DEBUG)
endif
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG=y
endif
ifeq ($(B_REFSW_DEBUG),y)
BCM_OBJ_DIR=$(B_REFSW_ARCH).debug$(NEXUS_BIN_DIR_SUFFIX)
else
BCM_OBJ_DIR=$(B_REFSW_ARCH).release$(NEXUS_BIN_DIR_SUFFIX)
endif

.PHONY: $(BCM_OBJ_DIR)
CP = cp -f

# use Kbuild makefile in BSEAV/linux/driver/usermode for 2.6.31
all install: $(BCM_OBJ_DIR)
	$(MAKE) -C ../../usermode ARCH=mips $@
	$(CP) ../../usermode/bcmdriver.ko $(BCM_OBJ_DIR)

$(BCM_OBJ_DIR):
	$(Q_)mkdir -p $(BCM_OBJ_DIR)

clean:
	$(MAKE) -C ../../usermode ARCH=mips $@
	$(RM) -r $(BCM_OBJ_DIR)

else

# for 2.6.18 and others, use our own module Makefile
.PHONY: checkdirs driver $(DRIVERS)
all: checkdirs driver
SYSTEM := linuxkernel
include Common.make
include ../../../../build/refsw_inc.mak
include ../usermode/usermode.mak

driver: checkdirs ${DRIVERS}

endif
