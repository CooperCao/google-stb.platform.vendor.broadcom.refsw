#############################################################################
#
#       Copyright (c) 2007-2013, Broadcom Corporation.
#       All rights reserved.
#       Confidential Property of Broadcom Corporation.
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# File Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################
# pmlib.inc
# This file should be included by applications linking to pmlib.
#
# Inputs: PMLIB_TOP
# Outputs: PMLIB_SRCS, PMLIB_CFLAGS
#
# Work out Kernel Version from Kernel Source's Top Level Makefile
#
K_VERSION       := $(shell grep -m1 "^VERSION" $(LINUX)/Makefile | cut -d\  -f3)
K_PATCHLEVEL    := $(shell grep -m1 "^PATCHLEVEL" $(LINUX)/Makefile | cut -d\  -f3)
K_SUBLEVEL      := $(shell grep -m1 "^SUBLEVEL" $(LINUX)/Makefile | cut -d\  -f3)
K_EXTRAVERSION  := $(shell grep -m1 "^EXTRAVERSION" $(LINUX)/Makefile | cut -d-  -f2-)
KERNEL_VERSION  := $(K_VERSION).$(K_PATCHLEVEL).$(K_SUBLEVEL)-$(K_EXTRAVERSION)

pmlib-check-linux-version = $(shell expr \( $(K_VERSION) \* 65536 + $(K_PATCHLEVEL) \* 256 + $(K_SUBLEVEL) \) \>= \( $(1) \* 65536 + $(2) \* 256 + $(3) \))


ifeq ($(call pmlib-check-linux-version,3,14,1),1)   # if kernel >= 3.14.1
    PMLIB_DIR=314
else
ifeq ($(call pmlib-check-linux-version,2,6,30),1)   # if kernel >= 2.6.30
    PMLIB_DIR=26
else                                    # if kernel < 2.6.30
    $(error PMlib not supported!!)
endif
endif

PMLIB_LIBDIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/pmlib
PMLIB_CFLAGS += -I$(PMLIB_TOP)/$(PMLIB_DIR)
PMLIB_CFLAGS += -DPMLIB_VER=$(PMLIB_DIR)
PMLIB_LDFLAGS += -L$(PMLIB_LIBDIR) -lpmlib
