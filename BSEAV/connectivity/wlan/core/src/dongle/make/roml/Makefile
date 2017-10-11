#
# Makefile for ROM libraries
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

# run silently by default
V ?= 0

# announce a few things everyone should know
ifeq ($(V),0)
export MAKE := $(MAKE) -s
endif

SUBMKFILES := $(wildcard */Makefile)
SUBDIRS	:= $(sort $(SUBMKFILES:/Makefile=))

all: $(SUBDIRS)

$(SUBDIRS): FORCE
	@$(MAKE) -C $@

clean: $(SUBDIRS:=_clean)

$(SUBDIRS:=_clean): FORCE
	@$(MAKE) -C $(@:_clean=) clean

.PHONY: FORCE
FORCE:
