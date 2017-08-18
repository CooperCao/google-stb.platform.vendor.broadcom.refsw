#
# DHD MacOS Makefile
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

SRCBASE = ../..
include $(SRCBASE)/Makerules

ifneq ($(TOPBLDDIR),)
  BUILD         := xcodebuild TOPBLDDIR=${TOPBLDDIR}
else
  BUILD         := xcodebuild TOPBLDDIR=builds
endif

ifdef ObjPfx
BUILD	        += OBJROOT=$(ObjPfx) SYMROOT=$(ObjPfx)
endif
PROJECT	        := BCM43xx.xcodeproj
SW_VER          := $(shell sw_vers -productVersion)
GALA_OS		:= $(strip $(filter 10.11%,$(SW_VER)))
SYRAH_OS	:= $(strip $(filter 10.10%,$(SW_VER)))
CAB_OS		:= $(strip $(filter 10.9%,$(SW_VER)))

# PCIWireless is default target
TARGET          ?= PCIWireless-DHD
# Additional Device targets
DEVICE_TARGETS  ?= PCIWireless-DHD

# Set PRODUCT name unique as per current device selected
ifeq ($(findstring PCI,$(TARGET)),PCI)
  PRODUCT       := AirPortBroadcom43XX
endif

KEXT	        := $(PRODUCT).kext

CFGS = Debug Release
AP_CFGS = Debug_AP Release_AP
MFG_CFGS = Debug_Mfg Release_Mfg

ifneq ($(CAB_OS),)
ifeq ($(SW_VER),10.9.1)
  DEBUGCFG   := Debug_10_10
  RELEASECFG := Release_10_10
  OS_SUFF := 10_10
  MFG_OS := 10_10
else
  DEBUGCFG   := Debug_10_9
  RELEASECFG := Release_10_9
  OS_SUFF := 10_9
  MFG_OS := 10_9
endif
  SDK_VER := -sdk macosx10.9
  BUILD += $(SDK_VER)
endif
ifneq ($(SYRAH_OS),)
  DEBUGCFG   := Debug_10_10
  RELEASECFG := Release_10_10
  OS_SUFF := 10_10
  MFG_OS := 10_10
  SDK_VER := -sdk macosx10.10
  BUILD += $(SDK_VER)
endif
ifneq ($(GALA_OS),)
  DEBUGCFG   := Debug_10_11
  RELEASECFG := Release_10_11
  OS_SUFF := 10_11
  MFG_OS := 10_11
  SDK_VER := -sdk macosx10.11
  BUILD += $(SDK_VER)
endif


DEFTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)))
MFGTARGETS = $(foreach cfgs, $(MFG_CFGS), $(foreach os, $(MFG_OS), $(cfgs)_$(os)))
KEXTTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)/$(KEXT)))
APKEXTTARGETS = $(foreach cfgs, $(AP_CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)/$(KEXT)))

# For each device, define a list of pseudo make targets
DEV_DEFTARGETS = $(foreach device, $(DEVICE_TARGETS), $(foreach defcfg, $(DEFTARGETS), $(device)_$(defcfg)))
DEV_MFGTARGETS = $(foreach device, $(DEVICE_TARGETS), $(foreach defcfg, $(MFGTARGETS), $(device)_$(defcfg)))

debug: $(DEBUGCFG)

release: $(RELEASECFG)

all: dhd_usb dhd_sdio $(DEFTARGETS)

all_devices: $(DEV_DEFTARGETS)

# Pseudo device targets that redirect to actual build config target
# by setting device type in TARGET variable, to match currrent build interface
$(DEV_DEFTARGETS) $(DEV_MFGTARGETS): TARGET=$(firstword $(subst _, ,$@))
$(DEV_DEFTARGETS) $(DEV_MFGTARGETS): DEFTARGET=$(subst $(TARGET)_,,$@)
$(DEV_DEFTARGETS) $(DEV_MFGTARGETS):
	@echo "Building $(DEFTARGET) for $(TARGET) device"
	$(MAKE) TARGET=$(TARGET) $(DEFTARGET)

$(P2PTARGETS): include brcmvirtether
	@echo "Building $@ project configuration now"
	$(BUILD) -project $(PROJECT) -target $(TARGET) -configuration $@ build

include:
	$(MAKE) -C ../../include

$(DEFTARGETS) $(MFGTARGETS) : include
	@echo "Building $@ project configuration now"
	$(BUILD) -project $(PROJECT) -target $(TARGET) -configuration $@ build
	THISUSER=`echo $$USER`; \
          if [[ $$THISUSER == hwnprebd || $$THISUSER == hwndebug || $$THISUSER == wlanswci ]] ; then \
            InfoFile=build/$@/AirPortBroadcom43XX.kext/Contents/Info.plist; \
          perl -p0777 -i -e 's#(CFBundle(Compatible)?Version</key>\n\t<string>)(.*)\<#$${1}1.1.1<#mg' $$InfoFile;\
	  fi

$(KEXTTARGETS):%/$(KEXT): %

$(APKEXTTARGETS):%/$(KEXT): %

package:
	$(MAKE) -C package

debug-package: debug
	$(MAKE) -C package debug

release-package: release
	$(MAKE) -C package release

all-package: all
	$(MAKE) -C package all

mfg:
	$(MAKE) $(MFGTARGETS)

dhd_usb:
	$(BUILD) -project $@.xcodeproj -target $@ -configuration Debug build

dhd_sdio:
	$(BUILD) -project $@.xcodeproj -target $@ -configuration Debug build

clean:
	$(foreach deftarget, $(DEFTARGETS), \
		$(BUILD) -project $(PROJECT) -target $(TARGET) -configuration $(deftarget) clean;)
	$(foreach deftarget, $(MFGTARGETS), \
		$(BUILD) -project $(PROJECT) -target $(TARGET) -configuration $(deftarget) clean;)
	rm -rf build

cleanall:
	rm -rf build
	$(MAKE) -C ../../include clean

.PHONY: all clean debug release $(DEFTARGETS) $(MFGTARGETS) package debug-package release-package
