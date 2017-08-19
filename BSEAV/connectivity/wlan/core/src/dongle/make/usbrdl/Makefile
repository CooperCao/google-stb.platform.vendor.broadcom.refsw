#
# Makefile for USB dongle bootloaders
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

SUBMKFILES := $(wildcard */Makefile)
# SUBDIRS	:= $(sort $(SUBMKFILES:/Makefile=))
SUBDIRS	:= 4373a0

all: $(SUBDIRS)

$(SUBDIRS): FORCE
	@$(MAKE) -C $@

mrproper: clean

clean: $(SUBDIRS:=_clean)

$(SUBDIRS:=_clean): FORCE
	@$(MAKE) -C $(@:_clean=) clean

.PHONY: FORCE
FORCE:
