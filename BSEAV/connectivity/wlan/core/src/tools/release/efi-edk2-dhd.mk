#
# EFI dhd build/release Makefile
#
# This Makefile is for building efi-dhd using edk2.0 toolchain on unix/linux environments
#
#  Copyright (C) 2017, Broadcom. All Rights Reserved.
#  
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted, provided that the above
#  copyright notice and this permission notice appear in all copies.
#  
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
#  SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
#  OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
#  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id$
#

BUILD_BASE := $(CURDIR)
FIND       := find
SRCFILTER   = perl src/tools/build/srcfilter.pl


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
TOOLSBIN  := /projects/hnd/tools/EFI/EDK2/EdkShellBinPkg/Bin/X64/Apps
NWKSTKBIN := /projects/hnd/tools/EFI/EDK2/Build/MdeModule/DEBUG_GCC44/X64

## Shared undefs between external and internal builds
UNDEFS := CONFIG_BCM93725 CONFIG_BCM93725_VJ CONFIG_BCM93730 \
          CONFIG_BCM933XX BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          BCM93725B BCM94100 BCM_USB NICMODE ETSIM INTEL NETGEAR \
          COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 \
          DSLCPE LINUXSIM BCMSIM BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 \
          MICROSOFT MSFT BCMSDIO POCKET_PC WLMOTOROLALJ WLNOKIA \
          BCMWAPI_WPI BCMWAPI_WAI
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

DEFS += EFI DHD_EFI

# for file list pre-processing
FILESDEFS = $(DEFS)
FILESUNDEFS =$(patsubst %,NO_%,$(UNDEFS))
FILESDEFS += FILESUNDEFS
GCCFILESDEFS = $(patsubst %,-D%_SRC,$(FILESDEFS))
# use only base includes

WLAN_GEN_BASEDIR=generated

.PHONY: all
all: build_start release build_end

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

# Unreleased-chiplist is included only for non-internal builds
ifeq ($(findstring BCMINTERNAL,$(DEFS))$(findstring internal,$(BRAND)),)
include unreleased-chiplist.mk
endif # BCMINTERNAL

.PHONY: build_start
build_start:
	@$(MARKSTART_BRAND)

.PHONY: filelists
filelists :
	@$(MARKSTART)
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	gcc -E -undef $(GCCFILESDEFS) -Ulinux -o $(SRCFILELIST) master_filelist.h
	@$(MARKEND)

define MOGRIFY_LIST
	$(FIND) src components $(MOGRIFY_SKIPCMD) -type f -print | perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' > src/.mogrified
endef

mogrify: filelists
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

src/.mogrified: filelists
	@$(MARKSTART)
	$(FIND) src components -name .cvsignore -exec rm -f {} +
	$(MAKE) -C src/include
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
	cp -R src/wl/exe/efi build/src/wl/exe
	cp -R src/dhd/exe/efi build/src/dhd/exe
	mkdir -p build/components/shared
	cp -R components/shared/devctrl_if/ build/components/shared
	cp -R components/shared/proto/ build/components/shared
	@$(MARKEND)

.PHONY: build
# Build from mogrified sources
build: build_src
	@$(MARKSTART)
	cd $(SRCDIR) && $(MAKE) uefidbg uefinodbg
	cd $(WLSRCDIR) && $(MAKE) uefidbg
	cd $(DHDSRCDIR) && $(MAKE) uefidbg
	@$(MARKEND)

.PHONY: release
# Install mogrified sources in the release directory
release: build
	@$(MARKSTART)
	$(FIND) src components | $(SRCFILTER) -v $(SRCFILELIST) | cpio -p -d release/$(RELDIR)/Dxe
	perl -wnpe 's/^(WL_BASE_PATH\s+=).*/$$1.\\src/' < release/$(RELDIR)/Dxe/src/dhd/efi/BcmDhd.inf > release/$(RELDIR)/Dxe/BcmDhd.inf
ifneq ($(findstring -external-,$(BRAND)),)
	$(RM) -r release/$(RELDIR)/Dxe/src/dhd/sys release/$(RELDIR)/Dxe/src/dhd/efi
endif
	mkdir -p release/$(RELDIR)/tools/X64
	install -p build/src/wl/exe/efi/X64/uefidbg/wl.efi release/$(RELDIR)/tools/X64
	install -p build/src/dhd/exe/efi/X64/uefidbg/dhd.efi release/$(RELDIR)/tools/X64
	install -p $(TOOLSBIN)/* release/$(RELDIR)/tools/X64
	install -p $(NWKSTKBIN)/* release/$(RELDIR)/tools/X64
	mkdir -p release/$(RELDIR)/efi/uefi64/dbg
	install -p build/src/dhd/efi/X64/uefidbg/BcmDhd.efi release/$(RELDIR)/efi/uefi64/dbg
	mkdir -p release/$(RELDIR)/efi/uefi64/nodbg
	install -p build/src/dhd/efi/X64/uefinodbg/BcmDhd.efi release/$(RELDIR)/efi/uefi64/nodbg
	cd release && tar -czf $(RELDIR)_$(rlsnum).tar.gz --exclude-vcs ./$(RELDIR)
	@$(MARKEND)

.PHONY: showenv
showenv:
	@$(MARKSTART)
	@echo "incremental = $(incremental)"
	@echo "RELDIR    = $(RELDIR)"
	@echo "BUILD_BASE  = $(BUILD_BASE)"
	@echo "CVSROOT     = $(CVSROOT)"
	@$(MARKEND)

#pragma runlocal
.PHONY: build_clean
build_clean: release
	@$(MARKSTART)
	-@$(FIND) src components -type f -iname "*\.obj" -o -iname "*\.o"  | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

.PHONY: build_end
build_end: build_clean
	@$(MARKEND_BRAND)
