#
# Makefile for Mac OS X
# Broadcom 802.11abg Networking Device Driver
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
#
# <<Broadcom-WL-IPTag/Private1743:>>
#
# $Id$
#

BUILD	:= xcodebuild
PROJECT	:= BrcmVirtEther.xcodeproj
#TARGET	:= Info
PRODUCT := BrcmVirtEther
KEXT	:= $(PRODUCT).kext

SW_VER     := $(shell sw_vers -productVersion)
SNOW_LEOPARD_OS	:= $(strip $(filter 10.6%,$(SW_VER)))
LEOPARD_OS	:= $(strip $(filter 10.5%,$(SW_VER)))
TIGER_OS	:= $(strip $(filter 10.4%,$(SW_VER)))

ifneq ($(SNOW_LEOPARD_OS),)
  DEBUGCFG   := BrcmVirtEther_Debug_10_6
  RELEASECFG := BrcmVirtEther_Release_10_6
endif
ifneq ($(LEOPARD_OS),)
  DEBUGCFG   := BrcmVirtEther_Debug_10_5
  RELEASECFG := BrcmVirtEther_Release_10_5
endif
ifneq ($(TIGER_OS),)
  DEBUGCFG   := BrcmVirtEther_Debug_10_4
  RELEASECFG := BrcmVirtEther_Release_10_4
endif

CFGS = BrcmVirtEther_Debug BrcmVirtEther_Release
OS_SUFF = 10_4 10_5 10_6

KEXTTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)))
DEFTARGETS = $(DEBUGCFG) $(RELEASECFG)

debug: $(DEFTARGETS)

all: $(KEXTTARGETS)

$(KEXTTARGETS):
	@echo "Building $@ project configuration now"
	$(BUILD) -project $(PROJECT)  -configuration $@ build

clean:
	$(foreach deftarget, $(KEXTTARGETS), \
		$(BUILD) -project $(PROJECT)  -configuration $(deftarget) clean;)

.PHONY: all clean debug release $(DEFTARGETS)
