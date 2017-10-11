#
# BCM947xx Linux 2.6  Internal Router build/release Makefile
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#

PARENT_MAKEFILE := linux-external-router-partial-src.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt

include linux-usbap.mk
include $(PARENT_MAKEFILE)

CONFIG_GLIBC = false

DEFS += BCMWPS
UNDEFS := $(filter-out _RTE_ BCMROMOFFLOAD BCMWPS,$(UNDEFS))
