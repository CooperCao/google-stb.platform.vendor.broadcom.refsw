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


.PHONY: checkdirs driver $(DRIVERS)
all: checkdirs driver

LINUX_INC = $(LINUX)/include

#
# The reference software does not automatically configure the kernel. That must be
# done by the developer.
#
ifeq ($(LINUX_VER_GE_2_6),y)
LINUX_VER_EQ_2_6_37 := $(shell (grep 'SUBLEVEL = 37' ${LINUX}/Makefile >/dev/null && echo 'y'))
endif

#ifneq ($(LINUX_VER_EQ_2_6_37),y)
#LINUX_CONFIGURED = $(shell test -f $(LINUX_INC)/linux/autoconf.h && echo "y")
#ifneq ($(LINUX_CONFIGURED),y)
#$(error $(LINUX) must be configured before using it to build a driver.)
#endif
#endif

ifdef PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)
endif

# set defaults
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG=y
endif
ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
endif

# reverse backward-compatibility for tools.mak and refsw_inc.mak
ifdef B_REFSW_DEBUG
DEBUG ?= $(B_REFSW_DEBUG)
endif
#ifdef B_REFSW_ARCH
#ARCH ?= $(B_REFSW_ARCH)
#endif
ifdef NEXUS_PLATFORM
PLATFORM ?= $(NEXUS_PLATFORM)
endif

# defines toolchain
#include tools.mak
#include ${BSEAV}/api/build/tools.mak

# destination for binary
ifeq ($(B_REFSW_DEBUG),y)
BCM_OBJ_DIR=$(B_REFSW_ARCH).debug
else
BCM_OBJ_DIR=$(B_REFSW_ARCH).release
endif

$(BCM_OBJ_DIR):
	$(Q_)mkdir -p $(BCM_OBJ_DIR)

checkdirs: $(BCM_OBJ_DIR) 


ifeq ($(B_REFSW_DEBUG),y)
CFLAGS += -DCONFIG_PROC_FS
endif

