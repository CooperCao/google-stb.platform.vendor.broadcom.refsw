#
# EFI dhd build/release Makefile
#
# This Makefile is intended to be run as part of windows environment
#
# Copyright (C) 2015 Broadcom Corporation
#
# $Id: efi-dhd.mk 616871 2016-02-03 09:28:43Z $
#

BUILD_BASE := $(CURDIR)
FIND       := find
SRCFILTER   = perl src/tools/build/srcfilter.pl

#SRCFILELISTS_COMPONENTS += src/tools/release/components/wlphy-filelist.txt
#SRCFILELISTS_COMPONENTS += src/tools/release/components/wlolpc-filelist.txt

SRCFILELIST_ORG = src/tools/release/efi-dhd-filelist.txt
SRCFILELIST = efi-dhd-filelist.txt
MOGRIFY     = perl src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))
export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
# MOGRIFY_FLAGS gets DEFS and UNDEFS and any mogrify.pl cmd line options
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
# Addition file types to mogrify in addition to what is in $(MOGRIFY_RULES)
export MOGRIFY_FILETYPES =
# Addition folders to skip in addition to what is in $(MOGRIFY_RULES)
export MOGRIFY_EXCLUDE   =

# These are module names and directories that will be checked out of CVS.
#
HNDSVN_BOM  = efi.sparse

# Symbols for mogrification come from the parent makefile calling this makefile

# How to filter the cvs diff for override patch
OVPATCH = perl src/tools/build/ovpatch.pl $(OVERRIDE)
OVFILE  = $(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)

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

RELDIR    := BRCM_Efi_Src
SRCDIR    := build/src/dhd/efi
WLSRCDIR  := build/src/wl/exe/efi
DHDSRCDIR := build/src/dhd/exe/efi
TOOLSBIN  := Z:/projects/hnd_cdroms/devtools/EFI/EDK_1_02/Tools


## Shared undefs between external and internal builds
UNDEFS := CONFIG_BCM93725 CONFIG_BCM93725_VJ CONFIG_BCM93730 CONFIG_BCM933XX BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS BCM93725B BCM94100 BCM_USB NICMODE ETSIM INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 DSLCPE LINUXSIM BCMSIM BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 MICROSOFT MSFT BCMSDIO POCKET_PC WLMOTOROLALJ WLNOKIA BCMWAPI_WPI BCMWAPI_WAI
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

DEFS += EFI  WLPHY DHD_EFI

# for file list pre-processing
FILESDEFS = $(DEFS)
FILESUNDEFS =$(patsubst %,NO_%,$(UNDEFS))
FILESDEFS += FILESUNDEFS
GCCFILESDEFS = $(patsubst %,-D%_SRC,$(FILESDEFS))
# use only base includes

WLAN_GEN_BASEDIR=generated

all: build_start release build_end

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

# Unreleased-chiplist is included only for non-internal builds
ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
endif # BCMINTERNAL

# Check out and mogrify sources
checkout: $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

filelists :
	@$(MARKSTART)
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	gcc -E -undef $(GCCFILESDEFS) -Ulinux -o $(SRCFILELIST) master_filelist.h
	@$(MARKEND)

define MOGRIFY_LIST
	$(FIND) src components $(MOGRIFY_SKIPCMD) -type f -print | perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' > src/.mogrified
endef

mogrify: checkout filelists
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

src/.mogrified: checkout filelists
	@$(MARKSTART)
	$(FIND) src components -name .cvsignore -exec rm -f {} +
	$(MAKE) -C src/include && echo src/include/epivers.h
	$(MOGRIFY_LIST)
	xargs $(MOGRIFY) < $@
	@$(MARKEND)

# Install mogrified sources in the build directory
build_src: src/.mogrified
	@$(MARKSTART)
	$(MAKE) -C src/include
	$(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST) | cpio -p -d build
# Copy this seperately as we don't want them to be released
	install -p src/include/epivers.h build/src/include
	install -p src/wl/config/wltunable_efi.h build/src/include/efi/wlconf.h
	install -p src/wl/config/wltunable_efi_acphy.h build/src/include/efi/wlconf_acphy.h
	install -p src/wl/config/wltunable_efi_4360.h build/src/include/efi/wlconf_4360.h
	install -p src/wl/config/wltunable_efi_4350.h build/src/include/efi/wlconf_4350.h
	install -p src/wl/config/wltunable_efi_43602.h build/src/include/efi/wlconf_43602.h
	cp -R src/wl/exe/efi build/src/wl/exe
	cp -R src/dhd/exe/efi build/src/dhd/exe
	mkdir -p build/components/shared
	cp -R components/shared/devctrl_if/ build/components/shared
	cp -R components/shared/proto/ build/components/shared
	$(FIND) build -type d -name CVS | xargs rm -rf
	@$(MARKEND)

# Build from mogrified sources
build: build_src
	@$(MARKSTART)
	cd $(SRCDIR) && $(MAKE) uefidbgallphy uefinodbgallphy uefidbgfwembedallphy uefinodbgfwembedallphy
	cd $(WLSRCDIR) && ./build.bat em64t
	cd $(DHDSRCDIR) && ./build.bat em64t
	@$(MARKEND)

# Install mogrified sources in the release directory
release: build
	@$(MARKSTART)
	$(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST) | cpio -p -d release/$(RELDIR)/Dxe
	perl -wnpe 's/^(WL_BASE_PATH\s+=).*/$$1.\\src/' < release/$(RELDIR)/Dxe/src/dhd/efi/BcmDhd.inf > release/$(RELDIR)/Dxe/BcmDhd.inf
	mkdir -p release/$(RELDIR)/tools/em64t/
	install -p build/src/wl/exe/efi/obj/em64t/bin/wl.efi release/$(RELDIR)/tools/em64t
	install -p build/src/dhd/exe/efi/obj/em64t/bin/dhd.efi release/$(RELDIR)/tools/em64t
	install -p $(TOOLSBIN)/em64t/* release/$(RELDIR)/tools/em64t
	rm -f release/$(RELDIR)/tools/em64t/dhcp-script.nsh
	mkdir -p release/$(RELDIR)/efi/uefi64/dbgallphy
	install -p build/src/dhd/efi/X64/uefi/dbgallphy/X64/BcmDhd.efi release/$(RELDIR)/efi/uefi64/dbgallphy
	mkdir -p release/$(RELDIR)/efi/uefi64/nodbgallphy
	install -p build/src/dhd/efi/X64/uefi/nodbgallphy/X64/BcmDhd.efi release/$(RELDIR)/efi/uefi64/nodbgallphy
	mkdir -p release/$(RELDIR)/efi/uefi64/nodbgfwembedallphy
	install -p build/src/dhd/efi/X64/uefi/nodbgfwembedallphy/X64/BcmDhd.efi release/$(RELDIR)/efi/uefi64/nodbgfwembedallphy
	mkdir -p release/$(RELDIR)/efi/uefi64/dbgfwembedallphy
	install -p build/src/dhd/efi/X64/uefi/dbgfwembedallphy/X64/BcmDhd.efi release/$(RELDIR)/efi/uefi64/dbgfwembedallphy
	$(FIND) release -type d -name CVS -o -name ".svn" | xargs rm -rf
	cd release && tar -czf $(RELDIR)_$(rlsnum).tar.gz --exclude=*/CVS --exclude=*/.svn ./$(RELDIR)
	@$(MARKEND)

showenv:
	@$(MARKSTART)
	@echo "incremental = $(incremental)"
	@echo "RELDIR    = $(RELDIR)"
	@echo "BUILD_BASE  = $(BUILD_BASE)"
	@echo "CVSROOT     = $(CVSROOT)"
	@$(MARKEND)

#pragma runlocal
build_clean: release
	@$(MARKSTART)
	-@$(FIND) src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)

.PHONY: all build release_src release
