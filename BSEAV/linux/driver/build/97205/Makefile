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
# $brcm_Log: $
# 
############################################################

BSEAV=$(shell cd ../../../..;pwd)

LINUX ?= /opt/brcm/linux
LINUX_VER_GE_2_6 ?= $(shell (grep 'PATCHLEVEL = 6' ${LINUX}/Makefile >/dev/null && echo 'y'))
ifeq ($(LINUX_VER_GE_2_6),y)
LINUX_VER_EQ_2_6_31 := $(shell (grep 'SUBLEVEL = 31' ${LINUX}/Makefile >/dev/null && echo 'y'))
endif

ifeq ($(LINUX_VER_EQ_2_6_31),y)

.PHONY: checkdirs driver $(DRIVERS)
all: checkdirs driver
include Common.2.6.31.make
include ../usermode/usermode.mak
driver: checkdirs $(DRIVERS)

else

# Check for SMP vs. non-SMP kernel configuration
ifeq ($(SMP),y)
DEFCONFIG = arch/mips/configs/bcm97405b0-smp_defconfig
else
DEFCONFIG = arch/mips/configs/bcm97405b0_defconfig
endif

include ../97400/Makefile

endif
