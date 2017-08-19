#
# Makefile for Mac OS X
# Broadcom 802.11abg Networking Device Driver Install Package
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

TARGET          ?= PCIWireless
#TARGET         ?= USBWireless

# Set PRODUCT name unique as per current device selected
ifeq ($(findstring PCI,$(TARGET)),PCI)
  PRODUCT       := AirPortBroadcom43XX
endif
ifeq ($(findstring USB,$(TARGET)),USB)
  PRODUCT       := AirPortBroadcomUSB43XX
endif

KEXT	        := $(PRODUCT).kext
PACKAGE	        := $(PRODUCT).pkg
P2PPACKAGE      := $(PRODUCT)_P2P.pkg

PACKAGE_FILES	:= package_Info.plist package_Description.plist resources/postflight resources/preupgrade

SW_VER          := $(strip $(shell sw_vers -productVersion))
SYRAH_OS	:= $(filter 10.10%,$(SW_VER))
CAB_OS          := $(filter 10.9%,$(SW_VER))
ZIN_OS          := $(filter 10.8%,$(SW_VER))
BAROLO_OS	:= $(filter 10.7%,$(SW_VER))
SNOW_LEOPARD_OS	:= $(filter 10.6%,$(SW_VER))
LEOPARD_OS	:= $(filter 10.5%,$(SW_VER))

ifneq ($(SYRAH_OS),)
  DEBUGCFG   := Debug_10_10
  RELEASECFG := Release_10_10
  DEBUGAPCFG   := Debug_AP_10_10
  RELEASEAPCFG := Release_AP_10_10
endif
ifneq ($(CAB_OS),)
ifeq ($(SW_VER),10.9.1)
  DEBUGCFG   := Debug_10_10
  RELEASECFG := Release_10_10
  DEBUGAPCFG   := Debug_AP_10_10
  RELEASEAPCFG := Release_AP_10_10
else
  DEBUGCFG   := Debug_10_9
  RELEASECFG := Release_10_9
  DEBUGAPCFG   := Debug_AP_10_9
  RELEASEAPCFG := Release_AP_10_9
endif
endif
ifneq ($(ZIN_OS),)
  DEBUGCFG   := Debug_10_8
  RELEASECFG := Release_10_8
  DEBUGAPCFG   := Debug_AP_10_8
  RELEASEAPCFG := Release_AP_10_8
endif
ifneq ($(BAROLO_OS),)
  DEBUGCFG   := Debug_10_7
  DEBUGAPCFG   := Debug_AP_10_7
  RELEASECFG := Release_10_7
  RELEASEAPCFG := Release_AP_10_7
  P2PPACKAGE :=
endif
ifneq ($(SNOW_LEOPARD_OS),)
  DEBUGCFG   := Debug_10_6
  RELEASECFG := Release_10_6
  DEBUGP2PCFG := Debug_P2P_10_6
  DEBUGVIRTIFCFG := Debug_VirtIf_10_6
endif
ifneq ($(LEOPARD_OS),)
  DEBUGCFG   := Debug_10_5
  RELEASECFG := Release_10_5
endif

CFGS = Debug Release
AP_CFGS = Debug_AP Release_AP
OS_SUFF = 10_5
MFG_CFGS = Debug_Mfg Release_Mfg
MFG_OS = 10_5

ifneq ($(SNOW_LEOPARD_OS),)
  OS_SUFF := $(OS_SUFF) 10_6
  MFG_OS := $(MFG_OS) 10_6
  P2P_OS_SUFF := P2P_10_6 VirtIf_10_6
endif
ifneq ($(BAROLO_OS),)
  OS_SUFF := 10_7
  MFG_OS := 10_7
  P2P_OS_SUFF :=
endif
ifneq ($(ZIN_OS),)
  OS_SUFF := 10_8
  MFG_OS := 10_8
  P2P_OS_SUFF :=
endif
ifneq ($(CAB_OS),)
ifeq ($(SW_VER),10.9.1)
  OS_SUFF := 10_10
  MFG_OS := 10_10
  P2P_OS_SUFF :=
else
  OS_SUFF := 10_9
  MFG_OS := 10_9
  P2P_OS_SUFF :=
endif
endif
ifneq ($(SYRAH_OS),)
  OS_SUFF := 10_10
  MFG_OS := 10_10
  P2P_OS_SUFF :=
endif

DEFTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)))
APTARGETS = $(foreach cfgs, $(AP_CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)))
MFGTARGETS = $(foreach cfgs, $(MFG_CFGS), $(foreach os, $(MFG_OS), $(cfgs)_$(os)))
P2PTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(P2P_OS_SUFF), $(cfgs)_$(os)))

P2PKEXTTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(P2P_OS_SUFF), ../build/$(cfgs)_$(os)/$(KEXT)))
KEXTTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(OS_SUFF), ../build/$(cfgs)_$(os)/$(KEXT)))
APKEXTTARGETS = $(foreach cfgs, $(AP_CFGS), $(foreach os, $(OS_SUFF), ../build/$(cfgs)_$(os)/$(KEXT)))
MFGKEXTTARGETS = $(foreach cfgs, $(MFG_CFGS), $(foreach os, $(MFG_OS), ../build/$(cfgs)_$(os)/$(KEXT)))

P2PPKGTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(P2P_OS_SUFF), $(cfgs)_$(os)/$(P2PPACKAGE)))
PKGTARGETS = $(foreach cfgs, $(CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)/$(PACKAGE)))
APPKGTARGETS = $(foreach cfgs, $(AP_CFGS), $(foreach os, $(OS_SUFF), $(cfgs)_$(os)/$(PACKAGE)))
MFGPKGTARGETS = $(foreach cfgs, $(MFG_CFGS), $(foreach os, $(MFG_OS), $(cfgs)_$(os)/$(PACKAGE)))
VIRTETHER=BrcmVirtEther

debug: $(DEBUGCFG)/$(PACKAGE)

release: $(RELEASECFG)/$(PACKAGE)

ap: $(DEBUGAPCFG)/$(PACKAGE)

ap-release: $(RELEASEAPCFG)/$(PACKAGE)

all: $(DEFTARGETS)

mfg: $(MFGPKGTARGETS)

p2p-all: $(P2PPKGTARGETS)

p2p: $(DEBUGP2PCFG)/$(P2PPACKAGE)

virtif: $(DEBUGVIRTIFCFG)/$(P2PPACKAGE)

$(DEFTARGETS):%: %/$(PACKAGE)
$(APTARGETS):%: %/$(PACKAGE)
$(MFGTARGETS):%: %/$(PACKAGE)
$(P2PTARGETS):%: %/$(P2PPACKAGE)
$(KEXTTARGETS) $(MFGKEXTTARGETS) $(P2PKEXTTARGETS) $(APKEXTTARGETS): ../build/%:
	make -C ../ $*

$(P2PPKGTARGETS): %/$(P2PPACKAGE): ../build/%/$(KEXT) $(PACKAGE_FILES)
	@if [ ! -d $* ]; then mkdir $*; fi
# Deprecate for all new OS versions
#ifeq ($(strip $(ZIN_OS) $(CAB_OS)),)
#	@./build_package.sh $< $@ $(VIRTETHER)
#endif

$(PKGTARGETS) $(MFGPKGTARGETS) $(APPKGTARGETS) : %/$(PACKAGE): ../build/%/$(KEXT) $(PACKAGE_FILES)
	@if [ ! -d $* ]; then mkdir $*; fi
# Deprecate for all new OS versions
#ifeq ($(strip $(ZIN_OS) $(CAB_OS)),)
#	@./build_package.sh $< $@
#endif

clean_pkg:
	-rm -rf $(DEFTARGETS) $(MFGTARGETS) $(P2PPKGTARGETS) $(APTARGETS)

.PHONY: all clean debug release $(DEFTARGETS) $(MFGTARGETS) $(P2PPKGTARGETS) $(APTARGETS)
