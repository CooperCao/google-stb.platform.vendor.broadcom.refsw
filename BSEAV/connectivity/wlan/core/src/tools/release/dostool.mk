#
# The DosTool release makefile
#
# Copyright 2005, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id$
#

PARENT_MAKEFILE :=
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

# There is no parent makefile for this
# include $(PARENT_MAKEFILE)

DATE       := $(shell date -I)
BUILD_BASE := $(shell pwd)
RELEASEDIR := $(BUILD_BASE)/release
PATH       := $(PATH):/projects/hnd/tools/djgpp/bin
UNAME      := $(shell uname -a)

#ifneq ($(findstring x86_64,$(UNAME)),)
#        $(error ERROR This $(BRAND) build can not run on 64bit os node ($(UNAME)). Exiting)
#endif # UNAME

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

ifneq ($(origin TAG), undefined)
    export TAG
    vinfo := $(subst _,$(space),$(TAG))
else
    vinfo := $(shell date '+D11 REL %Y %m %d')
endif

maj:=$(word 3,$(vinfo))
min:=$(word 4,$(vinfo))
rcnum:=$(word 5,$(vinfo))
rcnum:=$(patsubst RC%,%,$(rcnum))
ifeq ($(rcnum),)
  rcnum:=0
endif
incremental:=$(word 6,$(vinfo))
ifeq ($(incremental),)
  incremental:=0
endif
RELNUM:=$(maj).$(min).$(rcnum).$(incremental)
DOSTOOL_VER_SUFFIX:=$(subst .,_,$(RELNUM))

HNDSVN_BOM  := dostool.sparse

export TAG PATH

all: release
	date +"%c -- MARK $@ --"

# Build sources

build:
	date +"%c -- MARK build--"
	$(MAKE) -C src/wl/dos

# Archive the release
release: build
	date +"%c -- MARK release --"
	install -d $(RELEASEDIR)/dostool
	install src/wl/dos/cwsdpmi.exe \
	        $(RELEASEDIR)/dostool/cwsdpmi_$(DOSTOOL_VER_SUFFIX).exe
	cd $(RELEASEDIR)/dostool/; \
		ln -s cwsdpmi_$(DOSTOOL_VER_SUFFIX).exe cwsdpmi.exe
	install src/wl/dos/dost.exe \
	        $(RELEASEDIR)/dostool/dost_$(DOSTOOL_VER_SUFFIX).exe
	cd $(RELEASEDIR)/dostool/; \
		ln -s dost_$(DOSTOOL_VER_SUFFIX).exe dost.exe

# Clean targets
clean:
	$(MAKE) -C src/wl/dos clean
	rm -f $(RELEASEDIR)/dostool/cwsdpmi.exe
	rm -f $(RELEASEDIR)/dostool/dost.exe

.PHONY: all build release
