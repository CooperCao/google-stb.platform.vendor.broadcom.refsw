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
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BMRC

# Module Includes
BMRC_INCLUDES += $(MAGNUM)/commonutils/mrc/include

# Module Sources
ifeq ($(filter $(BCHP_CHIP), 3390 11360), $(BCHP_CHIP))
BMRC_STUB?=y
endif

ifeq ($(BMRC_STUB),y)
BMRC_SOURCES += $(MAGNUM)/commonutils/mrc/src/bmrc_stub.c
else
BMRC_SOURCES += $(MAGNUM)/commonutils/mrc/src/bmrc.c
endif

BMRC_SOURCES += $(MAGNUM)/commonutils/mrc/src/bmrc_clienttable_priv.c
BMRC_SOURCES += $(MAGNUM)/commonutils/mrc/src/bmrc_monitor_clients.c
BMRC_SOURCES += $(MAGNUM)/commonutils/mrc/src/bmrc_monitor.c


# By default, GFD and M2MC cannot access OS-allocated memory. They access MEM-allocated memory.
# If you have done the necessary work to provide contiguous physical memory with physical
# offsets, then you can allow GFD and M2MC to access OS-allocated memory. You must
# export BMRC_ALLOW_GFX_TO_ACCESS_KERNEL=y to prevent MRC from blocking this access.
#
ifeq ($(BMRC_ALLOW_GFX_TO_ACCESS_KERNEL),y)
BMRC_DEFINES += BMRC_ALLOW_GFX_TO_ACCESS_KERNEL
endif
ifeq ($(BMRC_ALLOW_XPT_TO_ACCESS_KERNEL),y)
BMRC_DEFINES += BMRC_ALLOW_XPT_TO_ACCESS_KERNEL
endif
ifeq ($(BMRC_ALLOW_M2M_TO_ACCESS_KERNEL),y)
# on 28nm, M2M SW is implemented with XPT HW, so you must use BMRC_ALLOW_XPT_TO_ACCESS_KERNEL
BMRC_DEFINES += BMRC_ALLOW_M2M_TO_ACCESS_KERNEL
endif
ifeq ($(BMRC_ALLOW_PCU_TO_ACCESS_KERNEL),y)
BMRC_DEFINES += BMRC_ALLOW_PCU_TO_ACCESS_KERNEL
endif
