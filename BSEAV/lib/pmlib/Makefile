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

# pmlib Makefile
#
SHELL=/bin/bash

include ../../api/build/tools.mak
BSEAV = $(shell cd "../.." && ${PWD})

# Identify object directory
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${BSEAV}/../${B_REFSW_OBJ_DIR}

.PHONY: check_environment static
all: check_environment static

PMLIB_TOP=$BSEAV/lib/pmlib
include pmlib.inc
include $(BSEAV)/build/refsw_inc.mak

ifeq ($(VERBOSE),)
MAKEFLAGS += -s
endif

LIB = pmlib

CFLAGS += -std=c99 -W -Wall
CFLAGS += $(B_REFSW_CFLAGS) $(B_REFSW_CFLAGS_SHARED) $(B_REFSW_GENERIC_MAGNUM_CFLAGS) $(B_REFSW_MAGNUM_INCLUDE_DIRS)

vpath %.c $(PMLIB_DIR)
SRCS = pmlib.c
CFLAGS += -I$(PMLIB_DIR)

ODIR = $(B_REFSW_OBJ_ROOT)/BSEAV/lib/pmlib

check_environment: $(ODIR)

include ../../api/build/rules.mak
