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
SW_VER_MAJOR    := $(word 1, $(subst ., ,$(SW_VER)))
SW_VER_MINOR    := $(word 2, $(subst ., ,$(SW_VER)))
OS_SUFF         := $(SW_VER_MAJOR)_$(SW_VER_MINOR)
OS_VER          := macosx$(SW_VER_MAJOR).$(SW_VER_MINOR)
SDK_VER         := -sdk $(OS_VER)
BUILD           += $(SDK_VER)

ifneq ("$(findstring $(SW_VER), 10.9 10.10)","")
  GENERIC_CONFIG := n
else
  GENERIC_CONFIG := y
endif

# PCIWireless is default target
TARGET          ?= PCIWireless
# Additional Device targets
DEVICE_TARGETS  ?= PCIWireless USBWireless

# Set PRODUCT name unique as per current device selected
ifeq ($(findstring PCI,$(TARGET)),PCI)
  PRODUCT       := AirPortBroadcom43XX
endif

ifeq ($(findstring USB,$(TARGET)),USB)
  PRODUCT       := AirPortBroadcomUSB43XX
endif

KEXT	        := $(PRODUCT).kext

CFGS = Debug Release
AP_CFGS = Debug_AP Release_AP
MFG_CFGS = Debug_Mfg Release_Mfg
DEBUGCFG = Debug_$(OS_SUFF)
RELEASECFG = Release_$(OS_SUFF)

# For each config, define a list of pseudo make targets
DEFTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)))
APTARGETS = $(foreach cfgs, $(AP_CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)))
MFGTARGETS = $(foreach cfgs, $(MFG_CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)))
KEXTTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)/$(KEXT)))
APKEXTTARGETS = $(foreach cfgs, $(AP_CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)/$(KEXT)))

# For each device, define a list of pseudo make targets
DEV_DEFTARGETS = $(foreach device, $(DEVICE_TARGETS), $(foreach defcfg, $(DEFTARGETS), $(device)_$(defcfg)))
DEV_MFGTARGETS = $(foreach device, $(DEVICE_TARGETS), $(foreach defcfg, $(MFGTARGETS), $(device)_$(defcfg)))
DEV_KEXTTARGETS = $(foreach device, $(DEVICE_TARGETS), $(foreach defcfg, $(KEXTTARGETS), $(device)_$(defcfg)))

XML_FILE = wlc_olyclm_data.xml
CLM_FILE = 43xx_macos.clm
CLM_DATA_FILE = wlc_clm_data.c

debug: $(DEBUGCFG)

release: $(RELEASECFG)

all: $(DEFTARGETS)

ifeq ($(GENERIC_CONFIG), y)
# Pseudo cfg targets that redirect to actual build config target
# by setting config type in TARGET variable, to match generic build config
$(DEFTARGETS) $(MFGTARGETS): CONFIG=$(firstword $(subst _10, ,$@))
else
$(DEFTARGETS) $(MFGTARGETS): CONFIG=$@
endif

all_devices: $(DEV_DEFTARGETS)

# Pseudo device targets that redirect to actual build config target
# by setting device type in TARGET variable, to match currrent build interface
$(DEV_DEFTARGETS) $(DEV_MFGTARGETS): TARGET=$(firstword $(subst _, ,$@))
$(DEV_DEFTARGETS) $(DEV_MFGTARGETS): DEFTARGET=$(subst $(TARGET)_,,$@)
$(DEV_DEFTARGETS) $(DEV_MFGTARGETS):
	@echo "Building $(DEFTARGET) for $(TARGET) device"
	$(MAKE) TARGET=$(TARGET) $(DEFTARGET)

brcmvirtether:
	$(MAKE) -C BrcmVirtEther

$(P2PTARGETS): include brcmvirtether
	@echo "Building $@ project configuration now"
	$(BUILD) -project $(PROJECT) -target $(TARGET) -configuration $@ build

include:
	$(MAKE) -C ../../include

clm:
	@echo "Building $(CLM_DATA_FILE) from $(XML_FILE) and $(CLM_FILE)"
	mkdir -p build/BCM43xx.build/$(OS_VER)
	if [[ -f ../../../components/clm-private/$(XML_FILE) ]]; then \
	    ../../../components/clm-bin/ClmCompiler.py --clmapi_include_dir ../../../components/clm-api/include --bcmwifi_include_dir ../../shared/bcmwifi/include --config_file ../clm/types/$(CLM_FILE) --obfuscate ../../../components/clm-private/$(XML_FILE) build/BCM43xx.build/$(OS_VER)/$(CLM_DATA_FILE); \
	else \
	    ../../../../components/clm-bin/ClmCompiler.py --clmapi_include_dir ../../../components/clm-api/include --bcmwifi_include_dir ../../shared/bcmwifi/include --config_file ../../../../src/wl/clm/types/$(CLM_FILE) --obfuscate ../../../../components/clm-private/$(XML_FILE) build/BCM43xx.build/$(OS_VER)/$(CLM_DATA_FILE); \
	fi;

sar:
	@echo "Building SAR tables"
	mkdir -p build/BCM43xx.build/$(OS_VER)
	if [[ -d ../../../src/wl/sar ]]; then \
		${python} ../../../components/tools/bin/sar_conv.py create_c --driver_dir ../../.. ../../../src/wl/sar/*.csv build/BCM43xx.build/$(OS_VER)/wlc_sar_tbl.c; \
	else \
		${python} ../../../../components/tools/bin/sar_conv.py create_c --driver_dir ../../.. ../../../../src/wl/sar/*.csv build/BCM43xx.build/$(OS_VER)/wlc_sar_tbl.c; \
	fi;

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(patsubst %/,%,$(dir $(mkfile_path)))
D11SHM_SRCBASE := $(current_dir)/../..
D11SHM_WOWL = 1
D11SHM_TEMPDIR := $(shell mktemp -d d11shm.XXXXXXXXXX)

wlconf.h:
	cp $(D11SHM_SRCBASE)/wl/config/wltunable_macos.h wlconf.h

include ../../makefiles/d11shm.mk

$(DEFTARGETS) $(MFGTARGETS) $(APTARGETS): include $(D11SHM_HEADER) clm sar
	@echo "Building $@ project configuration now"
	/bin/mkdir -p $(current_dir)/build/$(CONFIG)/include
	cp $(D11SHM_TEMPDIR)/* $(current_dir)/build/$(CONFIG)
	cp $(D11SHM_TEMPDIR)/* $(current_dir)/build/$(CONFIG)/include
	$(BUILD) -project $(PROJECT) -target $(TARGET) -configuration $(CONFIG) build
ifeq ($(GENERIC_CONFIG), y)
	cp -f build/BCM43xx.build/$(OS_VER)/$(CLM_DATA_FILE) build/$(OS_VER)/$(CONFIG)
	rm -rf build/$@
	cp -Rf build/$(OS_VER)/$(CONFIG) build/$@
	cp -Rf build/$(CONFIG)/$(KEXT).dSYM build/$@/
else
	cp -f build/BCM43xx.build/$(OS_VER)/$(CLM_DATA_FILE) build/$@
endif
ifeq ($(OS_VER), macosx10.11)
	@echo "Verifying link for build/$@/$(KEXT)"
	kextlibs -undef-symbols -- build/$@/$(KEXT)
endif

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

p2p:
	$(MAKE) $(DEBUGP2PCFG)
	$(MAKE) -C package p2p

virtif:  brcmvirtether
	$(MAKE) $(DEBUGVIRTIFCFG)
	$(MAKE) -C package virtif

p2p-all: brcmvirtether
	$(MAKE) $(P2PTARGETS)
	$(MAKE) -C package p2p-all

ap:
	$(MAKE) $(DEBUGAPCFG)
	$(MAKE) -C package ap

ap-release:
	$(MAKE) $(RELEASEAPCFG)
	$(MAKE) -C package ap-release

clean:
	$(foreach target, $(DEFTARGETS) $(MFGTARGETS), \
		rm -rf build/$(target);)
	$(foreach cfg, $(CFGS) $(MFGCFGS), \
		$(BUILD) -project $(PROJECT) -target $(TARGET) -configuration $(cfg) clean; \
		rm -rf build/$(OS_VER);)
	$(MAKE) -C package clean
	rm -rf d11*

cleanall:
	rm -rf build d11*
	$(MAKE) -C package clean
	$(MAKE) -C ../../include clean

.PHONY: all clean debug release $(DEFTARGETS) $(MFGTARGETS)  $(P2PTARGETS) $(APTARGETS) \
	package debug-package release-package
