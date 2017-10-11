#
# Linux External wl release source build/release Makefile
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

PARENT_MAKEFILE := linux-wl.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND          ?= linux-external-wl-full-src

RELEASE_WLSRC         := 1
RELEASE_WLEXESRC      := 1
RELEASE_BCMDLSRC      := 1
DRIVERS               := fc15
DRIVERS_64            :=
# Add only the lowest common for each arch from DRIVERS above to APP_DRVIERS
APP_DRIVERS           := fc15
APP_DRIVERS_64        :=
RELEASE_WLSRC_DRIVERS := fc15

fc15-DEFBASICS         := apdef-stadef-high

include $(PARENT_MAKEFILE)
