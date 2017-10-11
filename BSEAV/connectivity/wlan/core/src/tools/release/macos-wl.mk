#
# MacOS wl build/release global Makefile
#
# This Makefile is included by all macos build brands. All macos builds run
# on native MacOS machines.
#
# Copyright (C) 2004 Broadcom Corporation
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id: macos-wl.mk,v 1.21 2007/08/22 22:33:03 kiranm Exp
#

BUILD_BASE := $(PWD)
FIND       := find
PERL       := /usr/bin/perl
SRCFILTER   = $(PERL) src/tools/build/srcfilter.pl
export VCTOOL=svn
MOGRIFY     = $(PERL) src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))

SUBST       = $(PERL) src/tools/build/subst.pl
SUBSTLIST   = src/tools/release/linux-olympic-dongle-src-subst.txt

SRCFILELISTS_COMPONENTS += src/tools/release/components/wlphy-filelist.txt
SRCFILELIST_ORG          = src/tools/release/macos-filelist.txt
SRCFILELIST              = macos-filelist.txt

ALL_DNGL_IMAGES?=$(EMBED_DONGLE_IMAGES)
HNDRTE_IMGFN   :=rtecdc.h

HNDSVN_BOM     ?= wl-src.sparse

empty:=
space:= $(empty) $(empty)

ifneq ($(origin TAG), undefined)
    vinfo := $(subst _,$(space),$(TAG))
else
    vinfo := $(shell date '+DUMMY REL %Y %m %d')
endif

maj         := $(word 3,$(vinfo))
min         := $(word 4,$(vinfo))
rcnum       := $(word 5,$(vinfo))
rcnum       := $(patsubst RC%,%,$(rcnum))
ifeq ($(rcnum),)
  rcnum     := 0
endif
incremental := $(word 6,$(vinfo))
ifeq ($(incremental),)
   rlsnum    = $(maj)_$(min)_$(rcnum)
else
   rlsnum    = $(maj)_$(min)_$(rcnum)_$(incremental)
endif

IOKITDIR    := BRCM_IOKit_Src_$(rlsnum)
IOKITPASS   := brcm_iokit_src

# Default device target is PCI, unless overriden by a usb or sdio build brand
DEVICE_TARGET  ?= PCIWireless
SW_VER         := $(shell sw_vers -productVersion)

CFGS           ?= Debug Release
MFGCFGS        ?= Debug_Mfg Release_Mfg
APCFGS         ?= Debug_AP Release_AP

DEFTARGETS      = $(foreach cfg,$(CFGS),$(foreach os, $(OS_SUFF),$(cfg)_$(os)))
MFGTARGETS      = $(foreach cfg,$(MFGCFGS),$(foreach os,$(OS_SUFF),$(cfg)_$(os)))
APTARGETS       = $(foreach cfg,$(APCFGS),$(foreach os, $(OS_SUFF),$(cfg)_$(os)))

# All project configs to package
TARGETS      ?= $(DEFTARGETS) $(MFGTARGETS)
ifeq ($(findstring -highusb-,$(BRAND)),)
  TARGETS      += $(P2PTARGETS)
endif
ifneq (,$(findstring $(SW_VER), "10.9 10.10"))
ALL_CFGS     ?= $(TARGETS)
else
ALL_CFGS     ?= $(CFGS) $(MFGCFGS)
endif

## Common steps to copy over built objects
## Args = $1 is the project config name
define COPY_BUILT_OBJECTS
	@echo "#- $0"
	mkdir -p release/$(IOKITDIR)/AirPortBroadcom43XX/$1
	cp -Rpv build/src/wl/macos/build/$1/AirPortBroadcom43XX.kext  release/$(IOKITDIR)/AirPortBroadcom43XX/$1
endef # COPY_BUILT_OBJECTS

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES = [^\/]*\.mak
export MOGRIFY_EXCLUDE   =

# Common mogrification defs/undefs
# Specific symbols to mogrify in/out per brand come from calling makefile

UNDEFS := CONFIG_BCM93725 CONFIG_BCM93725_VJ CONFIG_BCM93730 CONFIG_BCM933XX BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS BCM93725B BCM94100 BCM_USB NICMODE ETSIM INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 DSLCPE LINUXSIM BCMSIM BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 MICROSOFT MSFT BCMSDIO POCKET_PC WLMOTOROLALJ WLNOKIA BCMWAPI_WPI BCMWAPI_WAI WLNOKIA_NVMEM
DEFS   := WLPHY
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

FILESDEFS = $(DEFS)
FILESUNDEFS =$(patsubst %,NO_%,$(UNDEFS))
FILESDEFS += FILESUNDEFS
GCCFILESDEFS = $(patsubst %,-D%_SRC,$(FILESDEFS))

all: build_start
all: showenv
all: checkout filelists
ifneq ($(ALL_DNGL_IMAGES),)
all: copy_dongle_images
endif # ALL_DNGL_IMAGES
all: mogrify substitute prebuild_prep build release
all: build_end

include $(MOGRIFY_RULES)
include $(BRAND_RULES)
include linux-dongle-image-launch.mk

# Unreleased-chiplist is included only for non-internal builds
ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
endif # BCMINTERNAL

# Check out the sources
checkout: $(CHECKOUT_TGT)
ifneq ($(findstring -highusb-,$(BRAND)),)
	@$(MARKSTART)
	sed -e 's/AirPortBroadcom43XX/AirPortBroadcomUSB43XX/g' \
		 src/wl/macos/package/resources/postflight > \
		 src/wl/macos/package/resources/postflight.new
	mv src/wl/macos/package/resources/postflight.new \
	   src/wl/macos/package/resources/postflight
	sed -e 's/AirPortBroadcom43XX/AirPortBroadcomUSB43XX/g' \
		 src/wl/macos/package/resources/preupgrade >  \
		 src/wl/macos/package/resources/preupgrade.new
	mv src/wl/macos/package/resources/preupgrade.new \
	   src/wl/macos/package/resources/preupgrade
	@$(MARKEND)
endif

build_start:
	@$(MARKSTART_BRAND)

filelists :
	# Temporary filelist generation for SVN build test
	@$(MARKSTART)
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	gcc -E -undef $(GCCFILESDEFS) -Ulinux -o $(SRCFILELIST) master_filelist.h
	@$(MARKEND)

define MOGRIFY_LIST
	$(FIND) . $(MOGRIFY_EXCLUDE) -type f | $(PERL) -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' > src/.mogrified
endef

mogrify: checkout
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS="components/phy"
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS="components/shared/proto"
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS="components/shared/devctrl_if"
	@$(MARKEND)

substitute: mogrify
	@$(MARKSTART)
	$(FIND) src components -type f \
	  | $(SUBST) $(SUBSTLIST)
	@$(MARKEND)

# Install mogrified sources in the release directory
prebuild_prep: substitute
	@$(MARKSTART)
	mkdir -p build/components/shared/ && \
	  cp -R components/shared/* build/components/shared/
	mkdir -p build/components/phy && \
	    cd components/phy && \
	    $(FIND) . \( -name .svn -o -name README -o -name unittest \) -prune -o -type f -print | \
	    pax -rw ../../build/components/phy
	$(FIND) src components \( -name .svn -o -name CVS -o -name unittest \) -prune -o -type f -print | \
	    $(SRCFILTER) -v $(SRCFILELIST) | \
	    pax -rw build
ifeq (,$(strip ${ALL_DNGL_IMAGES}))
	@echo ">>> NO DONGLE IMAGE HEADER FILES WILL BE COPIED!"
else
	@echo ">>> COPY HEADERS FOR IMAGES: $(notdir ${ALL_DNGL_IMAGES})"
	# EMBED_DONGLE_IMAGE header file is needed for subsequent steps
	mkdir -pv $(addprefix build/$(DNGL_IMGDIR)/,$(ALL_DNGL_IMAGES))
	mkdir -pv build/src/wl/sys
	@_OPATH=${BUILD_BASE}/build/src/wl/macos/build \
	 _IPATH=$(HNDRTE_DIR)/$(DNGL_IMGDIR) \
	 $(foreach i,${ALL_DNGL_IMAGES}, \
	   ; _OSTEM=$(firstword $(subst -, ,$(dir $i))) \
	   ; _ISTEM=$${_IPATH}/$i/rtecdc \
	   ; _IHDR=$${_ISTEM}_$${_OSTEM}.h \
	   ; _IDIS=$${_ISTEM}.dis \
	   ; _IMAP=$${_ISTEM}.map \
	   ; _IEXE=$${_ISTEM}.exe \
	   ; _ILDS=$${_ISTEM}.lds \
	     $(foreach t, \
	       $(if $(strip $(findstring /assert,$i) $(findstring -assert,$i)), \
	         $(filter Debug%,${ALL_CFGS}), \
	         $(filter Release%,${ALL_CFGS}) \
	        ), \
	       ; mkdir -p $${_OPATH}/$t/include \
	       ; _OHDR=$t/include/rtecdc_$${_OSTEM}.h \
	       ; _ODIS=$t/rtecdc_$${_OSTEM}.dis \
	       ; _OMAP=$t/rtecdc_$${_OSTEM}.map \
	       ; _OEXE=$t/rtecdc_$${_OSTEM}.exe \
	       ; _OLDS=src/tools/release/rtecdc_$${_OSTEM}.lds \
	       ; echo "DONGLE: install -pv $${_IHDR} $${_OPATH}/$${_OHDR}" \
	       ; install -pv $${_IHDR} $${_OPATH}/$${_OHDR} \
	       ; echo "DONGLE: install -pv $${_IDIS} $${_OPATH}/$${_ODIS}" \
	       ; install -pv $${_IDIS} $${_OPATH}/$${_ODIS} \
	       ; echo "DONGLE: install -pv $${_IMAP} $${_OPATH}/$${_OMAP}" \
	       ; install -pv $${_IMAP} $${_OPATH}/$${_OMAP} \
	       ; echo "DONGLE: install -pv $${_IEXE} $${_OPATH}/$${_OEXE}" \
	       ; install -pv $${_IEXE} $${_OPATH}/$${_OEXE} \
	       ; echo "DONGLE: install -pv $${_ILDS} $${_OLDS}" \
	       ; install -pv $${_ILDS} ${BUILD_BASE}/$${_OLDS} \
	       ; _OHDR=$(if $(filter Debug_%,$t),dbg,rel)-olbin_$${_OSTEM}.h \
	       ; _OHDR=src/wl/sys/$${_OHDR} \
	       ; echo "DONGLE: install -pv $${_IFILE} ${BUILD_BASE}/$${_OFILE}" \
	       ; install -pv $${_IHDR} ${BUILD_BASE}/$${_OHDR} \
	      ) \
	  )
	@echo ">>> FINISHED COPYING HEADERS"
endif # ALL_DNGL_IMAGES
	@$(MARKEND)

# Build from mogrified sources
build: prebuild_prep
	@$(MARKSTART)
	$(MAKE) -C build/src/wl/macos TARGET=$(DEVICE_TARGET) $(TARGETS)
	$(MAKE) -C build/src/wl/exe
	@$(MARKEND)

release: build
	@$(MARKSTART)
	mkdir -p release/$(IOKITDIR)/AirPortBroadcom43XX/Tools
	$(foreach target, $(TARGETS),  \
		mkdir -p release/$(IOKITDIR)/AirPortBroadcom43XX/$(target); \
		cp -Rpv build/src/wl/macos/build/$(target)/AirPortBroadcom43XX.kext  release/$(IOKITDIR)/AirPortBroadcom43XX/$(target); \
		cp -pv build/src/wl/macos/package/install.sh release/$(IOKITDIR)/AirPortBroadcom43XX/$(target); \
	)
	install build/src/wl/exe/macos/$(SW_VER)/wl release/$(IOKITDIR)/AirPortBroadcom43XX/Tools
	install build/src/wl/exe/macos/$(SW_VER)/socket/wl_server_socket release/$(IOKITDIR)/AirPortBroadcom43XX/Tools
	find src components \( -name .svn -o -name CVS -o -name unittest \) -prune -o -type f -print | $(SRCFILTER) -v $(SRCFILELIST) | \
	    pax -rw release/$(IOKITDIR)
	# Copy the wlc_clm_data.c file for external release
	mkdir -p release/$(IOKITDIR)/src/wl/macos/clm/
	cp build/src/wl/macos/build/Release_$(OS_SUFF)/wlc_clm_data.c \
	    release/$(IOKITDIR)/src/wl/macos/clm/
	cd release && tar -czf $(IOKITDIR).tar.gz --exclude='*/.svn' --exclude='*/.mogrified' ./$(IOKITDIR)
	@for n in 1 2 3 4 5; do \
	     echo "cd release && /bin/echo -n '$(IOKITPASS)' | hdiutil create -encryption -stdinpass -size 120m -srcfolder $(IOKITDIR) $(IOKITDIR).dmg"; \
	     cd release && /bin/echo -n "$(IOKITPASS)" | hdiutil create -encryption -stdinpass -size 120m -srcfolder $(IOKITDIR) $(IOKITDIR).dmg; \
	     ec=$$?; \
	     if [ ! -f "$(IOKITDIR).dmg" ]; then \
	        echo "WARN: hdiutil packaging failed. Retrying $n (exitcode:$$ec)"; \
	        sleep 5; \
	     else \
	        break; \
	     fi; \
	     cd ..; \
	done
	@if [ ! -f "release/$(IOKITDIR).dmg" ]; then \
	    echo "ERROR: $(IOKITDIR).dmg creation attempts failed"; \
	fi
	cd release && rm -rf $(IOKITDIR)
	@$(MARKEND)

showenv:
	@$(MARKSTART)
	@echo "BRAND       = $(BRAND)"
	@echo "PROJ-CONFIGS= $(DEBUGCFG) $(RELEASECFG)"
	@echo "IOKITDIR    = $(IOKITDIR)"
	@echo "BUILD SERVER SW_VER_INFO:"
	@sw_vers
	@$(MARKEND)

build_clean: release
#	Cleanup suppressed, as some of these files are needed for debugging
#	-@find build src -type f -name "*.obj" -o -name "*.OBJ" -o -name "*.o" | \
#	xargs rm -f

build_end: build_clean
	@$(MARKEND_BRAND)

.PHONY: all build release
