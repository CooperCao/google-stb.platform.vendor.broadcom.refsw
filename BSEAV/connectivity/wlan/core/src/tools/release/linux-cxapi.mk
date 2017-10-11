#
# Common release makefile to filter, prebuild and package
# broadcom linux wireless connection API
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

NULL               := /dev/null
FIND               := /usr/bin/find
BUILD_BASE         := $(shell pwd)
SRCDIR             := $(BUILD_BASE)/src
RELEASEDIR         := $(BUILD_BASE)/release
SRCFILTER          := perl $(SRCDIR)/tools/build/srcfilter.pl
UNAME              := $(shell uname -a)
OEM_LIST           ?= bcm
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE

SRCFILELISTS_COMPONENTS += src/tools/release/components/cpl/cxnn2-filelist.txt
SRCFILELIST             := generic-filelist.txt
SRCFILELIST_ORG         := $(SRCDIR)/tools/release/linux-cxapi-filelist.txt

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
# MOGRIFY_FLAGS gets DEFS and UNDEFS and any mogrify.pl cmd line options
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
# Addition file types to mogrify in addition to what is in $(MOGRIFY_RULES)
export MOGRIFY_FILETYPES =
# Addition folders to skip in addition to what is in $(MOGRIFY_RULES)
export MOGRIFY_EXCLUDE   =

HNDSVN_BOM         := cxnnapi.sparse

CXDIR              := src/wl/cpl/components/cxnn2

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $1/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $1/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

# Currently forcing this brand to build on 32bit nodes
ifneq ($(findstring x86_64,$(UNAME)),)
        $(error ERROR This $(BRAND) build can not run on 64bit os node ($(UNAME)). Exiting)
endif # UNAME

VALID_BRANDS:=linux-external-cxapi linux-internal-cxapi

ifeq ($(filter $(VALID_BRANDS),$(BRAND)),)
        $(error ERROR This $(BRAND) build is not a standalone build brand. Valid brands are $(VALID_BRANDS))
endif # BRAND

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

ifdef TAG
  vinfo := $(subst _,$(space),$(TAG))
else
  vinfo := $(shell date '+D11 REL %Y %m %d')
endif

maj     := $(word 3,$(vinfo))
min     := $(word 4,$(vinfo))
rcnum   := $(word 5,$(vinfo))
rcnum   := $(patsubst RC%,%,$(rcnum))
ifeq ($(rcnum),)
  rcnum := 0
endif
incr    := $(word 6,$(vinfo))
ifeq ($(incr),)
  incr  := 0
endif
RELNUM  := $(maj).$(min).$(rcnum).$(incr)

current_oem=$(word 2,$(subst _, ,$@))

ifneq ($(findstring x86_64,$(UNAME)),)
        32BIT := -32 ARCH=i386
        32GCC64 := CC='gcc -m32'
        32BITON64LIB := CROSS_LD_PATH=-L/projects/hnd/tools/linux/lib
        export 32ON64=1
endif # UNAME

GCC43X    := /tools/gcc/4.3.0/i686-2.6/bin/

#---------------------------------------------------------------
# Options for RH9.0 cxapi
rh90-cxapi-opts  := LINUXVER=2.4.20-8
rh90-cxapi-opts  += CROSS_COMPILE=/tools/gcc/3.3.1/i686-2.4/bin/
rh90-cxapi-opts  +=
# Build apps or utils if any for rh90
rh90-cxapi-app   := NATIVEBLD=1
#---------------------------------------------------------------

# From above list, choose os variants for each oem
DRIVERS_bcm      ?= fc15
DRIVERS_nobcmccx ?= fc15

# Generic hardware-less cxapi module make targets
CXAPI_TARGETS_bcm       := $(DRIVERS_bcm:%=bcm-%-cxapi-opts)
CXAPI_TARGETS_nobcmccx  := $(DRIVERS_nobcmccx:%=nobcmccx-%-cxapi-opts)

# These symbols will be UNDEFINED in the source code by the transmogirifier
UNDEFS += CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV \
          BCMSDIODEV WLNOKIA WLFIPS BCMPERFSTATS

# These symbols will be DEFINED in the source code by the transmogirifier
DEFS +=

# Combined mogrification in and out flags for specific oem needs (if any)
# generic bcm, everything inclusive
DEFS_bcm      += -DOEM_BCM
DEFS_bcm      += -UOEM_NOKIA
DEFS_bcm      += -UNOKIA
# non-bcmccx
DEFS_nobcmccx += -DOEM_BCM
DEFS_nobcmccx += -UOEM_NOKIA
DEFS_nobcmccx += -UNOKIA
DEFS_nobcmccx += -UBCMCCX -UBCMEXTCCX -DCX_API
# non-bcmcckm
DEFS_nobcmcckm += -DOEM_BCM
DEFS_nobcmcckm += -UOEM_NOKIA
DEFS_nobcmcckm += -UNOKIA
DEFS_nobcmcckm += -UBCMCCKM

all: build_start checkout mogrify build_include prebuild_prep build_cxapi release build_end

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

# How to filter the cvs diff for override patch
OVPATCH = perl src/tools/build/ovpatch.pl $(OVERRIDE)

filelists:
	@$(MARKSTART)
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
#disabled#	gcc -E -undef $(GCCFILESDEFS) -Ulinux -o $(SRCFILELIST) master_filelist.h
	# mimicking make_list behavior. In this case, the filelist isn't
	# filtered/pre-processed
	install master_filelist.h $(SRCFILELIST)
	@$(MARKEND)	

# check out files
checkout: $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

mogrify: checkout
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

build_include: checkout filelists mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)
## ------------------------------------------------------------------
## Build oem brand specific connectionAPI sub-build
## For each oem brand, a separate set of sources are filtered, mogrified
## and then cxapi is built
## ------------------------------------------------------------------
prebuild_prep: $(OEM_LIST:%=prebuild_%_prep)
	@echo "Done with $@"

$(OEM_LIST:%=prebuild_%_prep): oem=$(current_oem)
$(OEM_LIST:%=prebuild_%_prep): filelists
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,.)
	@echo "Copy over cxapi sources to $(oem) build workspace"
	mkdir -p build/$(oem)
	cp $(SRCFILELIST) $(SRCFILELIST)_$(oem)
	$(MOGRIFY) $(DEFS_$(oem)) $(SRCFILELIST)_$(oem)
	$(FIND) src components $(FIND_SKIPCMD) -print | $(SRCFILTER) -v $(SRCFILELIST)_$(oem) | \
		col -b | $(GREP_SKIPCMD) | \
		pax -rw $(PAX_SKIPCMD) build/$(oem)
	rm -f $(SRCFILELIST)_$(oem)
	$(MAKE) -f $(MOGRIFY_RULES) MOGRIFY_DIRS=build/$(oem)/src MOGRIFY_FLAGS="$(DEFS_$(oem))"

	# TODO: cxapi filelist and cvs module may need to be fixed to include
	install -d -m 755 build/$(oem)/src/include
	install src/include/epivers.h build/$(oem)/src/include/
	$(call REMOVE_WARN_PARTIAL_BUILD,.)
	@$(MARKEND)
# 	#End of prebuild_$(oem)_prep

## ------------------------------------------------------------------
## Build cxAPI in filtered directory
## ------------------------------------------------------------------
build_cxapi: $(OEM_LIST:%=build_%_cxapi)
	@echo "Done with $@"

build_bcm_cxapi: oem=bcm
build_bcm_cxapi: $(CXAPI_TARGETS_bcm)
	@echo "Done with $@"

build_nobcmccx_cxapi: oem=nobcmccx
build_nobcmccx_cxapi: $(CXAPI_TARGETS_nobcmccx)
	@echo "Done with $@"

#- Common cxapi build steps for all oems
#- This target can build cxapi for multiple linux os/variants
#- only if called modules can differentiate built bits based on linux os
$(foreach oem2,$(OEM_LIST),$(CXAPI_TARGETS_$(oem2))):
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,.)
	@echo "Build Options[$(oem)]='$($(subst $(oem)-,,$@))'"
	@if [ "$($(subst $(oem)-,,$@))" == "" ]; then \
	    echo "ERROR: $@ cxapi options missing"; \
	    echo "ERROR: Check CXAPI_TARGETS_$(oem) specification above"; \
	    exit 1; \
	fi
	$(MAKE) -C build/$(oem)/$(CXDIR)/linux \
		$(if $(BCM_MFGTEST), WLTEST=1) $($(subst $(oem)-,,$@))
	$(MAKE) -C build/$(oem)/$(CXDIR)/cnClient/linux \
		$(if $(BCM_MFGTEST), WLTEST=1) $($(subst $(oem)-,,$@))
	$(call REMOVE_WARN_PARTIAL_BUILD,.)
	@echo "============================================================="
	@$(MARKEND)

## ------------------------------------------------------------------
## Copy over built binaries to release/<oem> folders for each oem
## ------------------------------------------------------------------
release_bins: $(OEM_LIST:%=release_%_bins) $(OEM_LIST:%=release_%_extra_bins)

## ------------------------------------------------------------------
## First copy over *common* stuff to release/<oem> folders for each oem
## ------------------------------------------------------------------
$(OEM_LIST:%=release_%_bins): oem=$(current_oem)
$(OEM_LIST:%=release_%_bins): RELDIR=release/$(current_oem)
$(OEM_LIST:%=release_%_bins):
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,.)
	install -d -m 755 $(RELDIR)
	install -d -m 755 $(RELDIR)/bin
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	# Copy README and release notes (oem specific ones override generic)
	touch $(RELDIR)/README.txt
	-install src/doc/BrandReadmes/$(BRAND).txt $(RELDIR)/README.txt
	-install src/doc/BrandReadmes/$(BRAND)_oem_$(oem).txt $(RELDIR)/README.txt
	install src/doc/BCMLogo.gif $(RELDIR)
	install -p build/$(oem)/$(CXDIR)/bin/linux-native/cnClient $(RELDIR)/bin
	install -p build/$(oem)/$(CXDIR)/bin/linux-native/libcx.a $(RELDIR)/bin
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	$(call REMOVE_WARN_PARTIAL_BUILD,.)
# 	#End of release_$(oem)_bins
	@$(MARKEND)
## ------------------------------------------------------------------
## Copy additional files needed for cxapi package
## ------------------------------------------------------------------
$(OEM_LIST:%=release_%_extra_bins): oem=$(current_oem)
$(OEM_LIST:%=release_%_extra_bins): RELDIR=release/$(current_oem)
$(OEM_LIST:%=release_%_extra_bins):
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,.)
	install $(CXAPI_LXDIR)/usr/lib/libpcsclite.so       \
		$(CXAPI_LXDIR)/usr/lib/libpcsclite.so.1     \
		$(CXAPI_LXDIR)/usr/lib/libpcsclite.so.1.0.0 \
		$(RELDIR)/bin
	install $(CXAPI_LXDIR)/../redhat/RPMS/x86/CXAPI_PCTWIN_SIM_RPM_Packages \
		$(RELDIR)/bin
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	$(call REMOVE_WARN_PARTIAL_BUILD,.)
	@$(MARKEND)	
## ------------------------------------------------------------------
## Create oem brand specific source release package
## ------------------------------------------------------------------

release_src: $(OEM_LIST:%=release_%_src_package)

$(OEM_LIST:%=release_%_src_package): oem=$(current_oem)
$(OEM_LIST:%=release_%_src_package): RELDIR=release/$(current_oem)
$(OEM_LIST:%=release_%_src_package):
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,.)
	$(call WARN_PARTIAL_BUILD,$(RELDIR))
	@echo "Generating release source + binaries package now in $(RELDIR) ...."
	cp $(SRCFILELIST) $(SRCFILELIST)_$(oem)
	$(MOGRIFY) $(DEFS_$(oem)) $(SRCFILELIST)_$(oem)
	# Create a list of src files to filter first from SRCFILTER_<oem>
	cd build/$(oem) && $(FIND) src components | \
		$(SRCFILTER) -v $(BUILD_BASE)/$(SRCFILELIST)_$(oem) > pkg_these
	# Next create release/<oem>/src from SRCFILTER_<oem> src filter
	# pkg contents file pkg_these is retained (for any debugging needs)
	tar cpf - $(TAR_SKIPCMD) -C build/$(oem) -T build/$(oem)/pkg_these  | \
		 tar xpf - $(TAR_SKIPCMD) -C $(RELDIR)
	rm -f $(SRCFILELIST)_$(oem)
	tar cpf $(RELDIR)/linux-cxapi.tar $(TAR_SKIPCMD) -C $(RELDIR) \
	         src bin README.txt BCMLogo.gif
	gzip -9 $(RELDIR)/linux-cxapi.tar
	rm -rf $(RELDIR)/src
	$(call REMOVE_WARN_PARTIAL_BUILD,$(RELDIR))
	$(call REMOVE_WARN_PARTIAL_BUILD,.)
# 	#End of release_$(oem)_src_package
	@$(MARKEND)
release_build_clean: $(OEM_LIST:%=release_%_clean)

$(OEM_LIST:%=release_%_clean): oem=$(current_oem)
$(OEM_LIST:%=release_%_clean):
	@$(MARKSTART)
	@if [ -f "$(HNDRTE_FLAG)"   ]; then rm -fv  $(HNDRTE_FLAG);   fi
	@if [ -d "build/$(oem)"     ]; then \
	    echo "rm -rf build/$(oem)"; \
	    rm -rf build/$(oem); \
	    if [ "$(wildcard build/*)" == "build/$(oem)" ]; then \
	       rmdir -v build; \
	    fi; \
	fi
	@$(MARKEND)
release: release_bins release_src release_build_clean

build_clean: release
	@$(MARKSTART)	
	-@find src components build -type f -name "*.o" -o -name "*.obj" | xargs rm -f
	@$(MARKEND)

build_end: build_clean
	@rm -fv $(HNDRTE_FLAG)
	@$(MARKEND_BRAND)

.PHONY: FORCE checkout filelists build_end prebuild_prep $(OEM_LIST:%=build_%_cxapi) release release_bins release_src $(OEM_LIST:%=release_%_src_package) $(OEM_LIST:%=release_%_bins)
