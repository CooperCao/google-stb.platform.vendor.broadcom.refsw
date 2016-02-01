############################################################
#     Copyright (c) 2003-2014, Broadcom Corporation
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

# use Kbuild makefile in BSEAV/linux/driver/usermode for 2.6.31
all install clean:
	$(MAKE) -C ../../usermode ARCH=mips $@

else

# for 2.6.18 and others, use our own module Makefile
B_REFSW_ARCH ?= mipsel-linux
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG=y
endif

CWD := $(shell pwd)
B_REFSW_ROOT := ${CWD}/../../../../..

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_ROOT}/${B_REFSW_OBJ_DIR}
BCM_OBJ_DIR := ${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/usermode/$(B_REFSW_ARCH)

.PHONY: checkdirs driver $(DRIVERS)
CP = cp -rf

all: checkdirs driver
SYSTEM := linuxkernel
include Common.make
include ../../../../build/refsw_inc.mak
include ../usermode/usermode.mak

driver: checkdirs ${DRIVERS}

install: all

endif
