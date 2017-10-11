#
# Linux Mfgtest wl build/release Makefile
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

PARENT_MAKEFILE := linux-external-wl.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND             ?= linux-mfgtest-wl

export BCM_MFGTEST       := true
export RELEASE_WLEXESRC  ?= 1

DRIVERS        ?= fc15
# Add only the lowest common for each arch from DRIVERS above to APP_DRVIERS
APP_DRIVERS    ?= fc15

fc15-DEFBASICS   ?= apdef-stadef apdef-stadef-high apdef-stadef-extnvm

include $(PARENT_MAKEFILE)
