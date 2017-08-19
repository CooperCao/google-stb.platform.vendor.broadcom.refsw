#
# Build WinCE DHD on windows and SDIO Dongle Image (on linux) and package
# them together as an self installable .exe with ezsetup
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

SHELL                 := bash.exe
MAKE_MODE             := unix
NULL                  := /dev/null
BUILD_BASE            := $(shell pwd)
RELEASEDIR            := $(BUILD_BASE)/release
BRAND                 ?= ce_external_dongle_sdio
OVFILE                 = $(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)
ifdef BCM_MFGTEST
  WLTEST               = 1
endif
export ECADDIN_DONT_RM_TMP_MAKEFILES=1
override ECADDIN_MAX_PDB_FILES=

WLAN_WINPFX           ?= Z:
SRCFILTER             := $(BUILD_BASE)/src/tools/build/srcfilter.pl
SRCFILELIST_ORG           := $(BUILD_BASE)/src/tools/release/ce-dhd-filelist.txt
SRCFILELIST           := ce-dhd-filelist.txt
MOGRIFY               := $(BUILD_BASE)/src/tools/build/mogrify.pl
TREECMD               := $(firstword $(wildcard c:/tools/utils/tree.exe d:/tools/utils/tree.exe $(WLAN_WINPFX)/projects/hnd/tools/win/utils/tree.exe))

## These are build types in src. For release packaging 'free' objects are used
DEVTTYPES             ?= checked free
DEVNAMES              ?= bcm
DEVTYPES              ?= sddhd
DEVOSVERS_all         ?= 500 600
DEVPROCS_all          ?= X86 ARM

# oem or device specific configurations
DEVPROCS_bcm          := $(DEVPROCS_all)
DEVPROCS_qphone       := ARM
DEVOSVERS_bcm         := $(DEVOSVERS_all)
DEVOSVERS_qphone      := $(DEVOSVERS_all)
CE_REDIST_DIR         := $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/WinCE-Redist
MSI_ce500_ARM         := $(CE_REDIST_DIR)/500/ARM_500_SDK.msi
MSI_ce500_X86         := $(CE_REDIST_DIR)/500/X86_500_SDK.msi
MSI_ce600_ARM         := $(CE_REDIST_DIR)/600/ARM_600_SDK.msi
MSI_ce600_X86         := $(CE_REDIST_DIR)/600/X86_600_SDK.msi

# EAP plugins and their destination paths in release package
RATTLS_DIR               := src/wl/cpl/components/sup/eapttls/ttls_raseap
RATTLS_DLL_STDSDK500_ARM := rattls_stdsdk500_arm.dll
RATTLS_PDB_STDSDK500_ARM := rattls_stdsdk500_arm.pdb
RATTLS_DLL_WM6SDK_ARM    := rattls_wm6_arm.dll
RATTLS_PDB_WM6SDK_ARM    := rattls_wm6_arm.pdb
RATTLS_DLL_x86           := rattls_x86.dll
RATTLS_PDB_x86           := rattls_x86.pdb
RATTLS_DLL_x64           := rattls_x64.dll
RATTLS_PDB_x64           := rattls_x64.pdb
RASIM_DIR                := src/wl/cpl/components/sup/eapsim/sim_raseap
RASIM_DLL_STDSDK500_ARM  := rasim_stdsdk500_arm.dll
RASIM_PDB_STDSDK500_ARM  := rasim_stdsdk500_arm.pdb
RASIM_DLL_WM6SDK_ARM     := rasim_wm6_arm.dll
RASIM_PDB_WM6SDK_ARM     := rasim_wm6_arm.pdb
RASIM_DLL_x86            := rasim_x86.dll
RASIM_PDB_x86            := rasim_x86.pdb
RASIM_DLL_x64            := rasim_x64.dll
RASIM_PDB_x64            := rasim_x64.pdb
RAAKA_DIR                := src/wl/cpl/components/sup/eapaka/aka_raseap
RAAKA_DLL_STDSDK500_ARM  := raaka_stdsdk500_arm.dll
RAAKA_PDB_STDSDK500_ARM  := raaka_stdsdk500_arm.pdb
RAAKA_DLL_WM6SDK_ARM     := raaka_wm6_arm.dll
RAAKA_PDB_WM6SDK_ARM     := raaka_wm6_arm.pdb
RAAKA_DLL_x86            := raaka_x86.dll
RAAKA_PDB_x86            := raaka_x86.pdb
RAAKA_DLL_x64            := raaka_x64.dll
RAAKA_PDB_x64            := raaka_x64.pdb

# Build both release and debug project configs
PROJECT_CONFIGS_EAP      := 'Release|STANDARDSDK_500 (ARMV4I)'
PROJECT_CONFIGS_EAP      += 'Release|Windows Mobile 6 Professional SDK (ARMV4I)'
PROJECT_CONFIGS_EAP      += 'Debug|STANDARDSDK_500 (ARMV4I)'
PROJECT_CONFIGS_EAP      += 'Debug|Windows Mobile 6 Professional SDK (ARMV4I)'

EMBED_DONGLE_IMAGE  ?=

ALL_DNGL_IMAGES        = $(EMBED_DONGLE_IMAGE)
HNDRTE_DEPS           := checkout
HNDRTE_IMGFN          := rtecdc.h

# List of all valid dhd/exe build target combinations
ALL_BTARGETS    := $(foreach devtype,$(DEVTYPES),\
                   $(foreach devname,$(DEVNAMES),\
                   $(foreach devproc,$(DEVPROCS_$(devname)),\
                   $(foreach devos,$(DEVOSVERS_$(devname)),\
                   ce$(devos)_$(devproc)_$(devtype)_$(devname)))))

# For release, use only 'free' versions, although 'checked' are built under src
# List of all valid releasedir target combinations
ALL_RTARGETS    := $(foreach btgt,$(ALL_BTARGETS),$(btgt)_free)

# List of all valid source release target combinations
ALL_STARGETS    := $(foreach devname,$(DEVNAMES),\
                   rlssrc_$(devname))

## Following *_TARGETS can be subsets of above valid targets
BUILD_TARGETS     := $(ALL_BTARGETS)
RLS_TARGETS       := $(ALL_RTARGETS)
SRC_TARGETS       := $(ALL_STARGETS)
RLS_EAP_TARGETS   := ce500_ARM_bcm

## Dynamically derive a list of subtargets for source verification
CUR_BTARGETS       = $(strip $(foreach tgt,$(filter %$(DEVNAME),$(BUILD_TARGETS)),$(if $(findstring $(DEVPROC),$(tgt)),$(tgt))))

ifeq ($(DEVTTYPES),free)
  export TTYPE    := OPT
endif # DEVTTYPES

ifeq ($(DEVTTYPES),checked)
  export TTYPE    := DBG
endif # DEVTTYPES

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

ifneq ($(origin TAG), undefined)
    export TAG
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

# Copy over built EAP Plugin to release folder
#
# Targets that are needed for eap type release or for internal debugging
# 1. Copy over microsoft redist dlls (atl80.dll,msvcr80.dll,MFC80U.dll)
# 2. Next purge any existing ra<EAP_TYPE>.dll
# 3. Copy ra<EAP_TYPE>.dll with project config prefix (for both stdsdk and wm)
# 4. For Debug config, copy over ra<EAP_TYPE>.pdb file as well
# ARGS to this func:
#
# $1 = rattls or rasim or raaka; $2 = X86/ARM; $3 = Release or Debug
define COPY_EAP_PLUGINS
	@echo "#- $0($1,$2,$3)"
	@if [ "$(2)" == "ARM" ]; then \
	   install -pv $($(1)_DIR)/standardsdk_500_armv4i/$3/$(DEVNAME)/*.dll \
	   	       $($(1)_DIR)/standardsdk_500_armv4i/$3/$(DEVNAME)/*.DLL \
		    release/$(RDIR)/$(3); \
	   rm -fv   release/$(RDIR)/$(3)/$(1).dll; \
	   install -pv $($(1)_DIR)/standardsdk_500_armv4i/$3/$(DEVNAME)/$(1).dll \
		    release/$(RDIR)/$(3)/$($(1)_DLL_STDSDK500_$(DEVPROC)); \
	   install -pv $($(1)_DIR)/windows_mobile_6_professional_sdk_armv4i/$3/$(DEVNAME)/$(1).dll \
		    release/$(RDIR)/$(3)/$($(1)_DLL_WM6SDK_$(DEVPROC)); \
	   if [ "$(findstring Debug,$3)" != "" ]; then \
		install -pv $($(1)_DIR)/standardsdk_500_armv4i/$3/$(DEVNAME)/$(1).pdb \
			release/$(RDIR)/$(3)/$($(1)_PDB_STDSDK500_$(DEVPROC)); \
	   	install -pv $($(1)_DIR)/windows_mobile_6_professional_sdk_armv4i/$3/$(DEVNAME)/$(1).pdb \
			release/$(RDIR)/$(3)/$($(1)_PDB_WM6SDK_$(DEVPROC)); \
	   fi; \
	else \
	   install -pv $($(1)_DIR)/$3/$(DEVNAME)/*.dll \
	   	       $($(1)_DIR)/$3/$(DEVNAME)/*.DLL \
		    release/$(RDIR)/$(3); \
	   rm -fv   release/$(RDIR)/$(3)/$(1).dll; \
	   install -pv $($(1)_DIR)/$3/$(DEVNAME)/$(1).dll \
		    release/$(RDIR)/$(3)/$($(1)_DLL_x86); \
	   install -pv $($(1)_DIR)/x64/$3/$(DEVNAME)/$(1).dll \
		    release/$(RDIR)/$(3)/$($(1)_DLL_x64); \
	   if [ "$(findstring Debug,$3)" != "" ]; then \
	   	install -pv $($(1)_DIR)/$3/$(DEVNAME)/$(1).pdb \
			release/$(RDIR)/$(3)/$($(1)_PDB_x86); \
	   	install -pv $($(1)_DIR)/x64/$3/$(DEVNAME)/$(1).pdb \
			release/$(RDIR)/$(3)/$($(1)_PDB_x64); \
	   fi; \
	fi
endef # COPY_EAP_PLUGINS

HNDCVS_BOM   := hndrte-bom
HNDCVS_DEFS  := -defs CE
HNDCVS         = PATH="/usr/bin:$(shell cygpath -u $(WLAN_WINPFX)/home/hwnbuild/src/tools/build):$(PATH)" && hndcvs

# These symbols will be UNDEFINED in the source code by the transmogirifier
#
# NOTE: Since this is a global makefile, include only gloabl DEFS and UNDEFS
# here. Device and Build type specific flags should go to individual (calling)
# makefile (e.g: ce_pcmcia_external_wl.mk)
#
UNDEFS += CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV \
          BCMCCX BCMEXTCCX BCMSDIODEV BCMSDIONP \
          BCMINTERNAL SDNOW_UNIT_TEST AIROPEEK WLFIPS \
          WLNOKIA BCMWAPI_WPI BCMP2P BCMWAPI_WAI \
          LINUX linux DHD_GPL BCMSLTGT BCMSPI OEM_ANDROID \
          BCMJTAG NOTYET \
          _CFE_ _RTE_ BCMECICOEX \
          BCMROMOFFLOAD

ifndef WLTEST
  UNDEFS += WLTEST
endif

# These symbols will be DEFINED in the source code by the transmogirifier
#
DEFS += BCM47XX BCM47XX_CHOPS BCMSDIO BCMCRYPTO BCMDHD WINCE \
        UNDER_CE BCMDONGLEHOST

# for file list pre-processing
FILESDEFS = $(DEFS)
FILESUNDEFS =$(patsubst %,NO_%,$(UNDEFS))
FILESDEFS += FILESUNDEFS
GCCFILESDEFS = $(patsubst %,-D%_SRC,$(FILESDEFS)) -DWLPHY_INC -DBCMDBG_SRC

# These are extensions of source files that should be transmogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

all: profile.log checkout mogrify showinfo build_include copy_dongle_images build_eap_plugins build_apps build_dhd release release_src build_end

include linux-dongle-image-launch.mk
include unreleased-chiplist.mk
include swrls.mk

showinfo:
	@echo "----------------------------------------------------"
	@echo "BUILD_TARGETS    = $(strip $(BUILD_TARGETS))"
	@echo "RLS_TARGETS      = $(strip $(RLS_TARGETS))"
	@echo "SRC_TARGETS      = $(strip $(SRC_TARGETS))"
	@echo "ALL_DNGL_IMAGES  = $(ALL_DNGL_IMAGES)"
	@echo "----------------------------------------------------"

profile.log :
	@date +"START: $(BRAND) tag=${TAG} %D %T" | tee profile.log

# check out files
checkout: profile.log
	@uname -a
	@date +"START: $@ $(HNDCVS_BOM), %D %T"  | tee -a profile.log
ifneq ($(SRC_CHECKOUT_DISABLED),1)
	$(HNDCVS) $(HNDCVS_DEFS) -new_make $(MAKE) \
		$(if $(CVSCUTOFF),-date "$(CVSCUTOFF)") \
		$(HNDCVS_BOM) $(TAG)
	@date +"END: $@ $(HNDCVS_BOM), %D %T"  | tee -a profile.log
	rm -rf src/wl/locale/*/images/dell
	rm -rf src/wl/locale/*/images/hp
endif # SRC_CHECKOUT_DISABLED
	@date +"END:   $@, %D %T"  | tee -a profile.log

filelists :
# make master list
	./src/hndcvs/make_list.sh -i $(SRCFILELIST_ORG) -o master-list $(HNDCVS_DEFS) -ml modules-list-bom $(TAG)
# filter master list to produce individual lists
	./src/hndcvs/make_list.sh  -f -defs "$(GCCFILESDEFS)" -i master-list -o $(SRCFILELIST)

mogrify: filelists src/.mogrified

src/.mogrified :
	@date +"START: $@, %D %T" | tee -a profile.log
	/usr/bin/find src components $(MOGRIFY_EXCLUDE) -type f -print  |  \
		perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' | \
		xargs perl $(MOGRIFY) $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))
	touch src/.mogrified
	@date +"END: $@, %D %T" | tee -a profile.log

build_include: checkout
	@date +"START: $@, %D %T"  | tee -a profile.log
	$(MAKE) -C src/include
	@date +"END:   $@, %D %T"  | tee -a profile.log

## Build eap plugins (ttls,sim,aka)
build_eap_plugins: checkout mogrify
	@date +"START: $@, %D %T" | tee -a profile.log
	$(MAKE) -C src/wl/cpl/components/sup/eapttls/ttls_raseap \
	         PROJECT_CONFIGS="$(PROJECT_CONFIGS_EAP)"
	$(MAKE) -C src/wl/cpl/components/sup/eapsim/sim_raseap \
	         PROJECT_CONFIGS="$(PROJECT_CONFIGS_EAP)"
	$(MAKE) -C src/wl/cpl/components/sup/eapaka/aka_raseap \
	         PROJECT_CONFIGS="$(PROJECT_CONFIGS_EAP)"
	@date +"END:   $@, %D %T"  | tee -a profile.log

build_apps: checkout mogrify
	@date +"START: $@, %D %T" | tee -a profile.log
	$(MAKE) -C src/wl/exe/ce \
		$(if $(WLTEST),WLTEST=1) \
		TARGETS="$(BUILD_TARGETS)" ce_exe
	$(MAKE) -C src/dhd/exe/ce \
		$(if $(WLTEST),WLTEST=1) \
		TARGETS="$(BUILD_TARGETS)" ce_exe
	@date +"END: $@, %D %T" | tee -a profile.log

build_dhd: checkout mogrify copy_dongle_images
	@date +"START: $@, %D %T"  | tee -a profile.log
	$(MAKE) -C src/dhd/ce \
		$(if $(WLTEST),WLTEST=1) \
		TARGETS="$(BUILD_TARGETS)" ce_dhd
	gmake -C src/dhd/ce/install \
		$(if $(WLTEST),WLTEST=1) \
		TARGETS="$(BUILD_TARGETS)" TTYPES="$(DEVTTYPES)"
	@date +"END:   $@, %D %T"  | tee -a profile.log

clean:
	$(MAKE) -C src/wl/exe/ce clean_exe
	$(MAKE) -C src/dhd/exe/ce clean_exe
	$(MAKE) -C src/dhd/ce clean_dhd
	$(MAKE) -C src/dhd/ce/install clean

pre_release:
	mkdir -p $(RELEASEDIR)
	# Install misc and create dirs
	@for oem in $(DEVNAMES); do \
		install -dv release/$${oem}/apps \
		         release/$${oem}/driver \
		         release/$${oem}/firmware \
		         release/$${oem}/installer; \
		install -pv src/doc/BCMLogo.gif release/$${oem}/; \
		echo "Updating releasenumber in ReleaseNotes.htm"; \
		sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
			src/doc/ReleaseNotesDongleSdio.html > \
			release/$${oem}/ReleaseNotes.htm; \
	done

pkg_release: $(RLS_TARGETS) $(RLS_EAP_TARGETS)

# Example rls tgt: $(devtype)_ce$(devos)_$(devproc)_$(devname)_$(ttype)
$(RLS_TARGETS): DEVOS=$(subst ce,,$(word 1,$(subst _,$(space),$(@))))
$(RLS_TARGETS): DEVPROC=$(word 2,$(subst _,$(space),$(@)))
$(RLS_TARGETS): DEVTYPE=$(word 3,$(subst _,$(space),$(@)))
$(RLS_TARGETS): DEVNAME=$(word 4,$(subst _,$(space),$(@)))
$(RLS_TARGETS): TTYPE=$(word 5,$(subst _,$(space),$(@)))
$(RLS_TARGETS): BDIR=$(DEVOS)/$(DEVPROC)/$(TTYPE)
$(RLS_TARGETS): checkout mogrify
$(RLS_TARGETS):
	@date +"START: $@, %D %T"  | tee -a profile.log
	# Install Firmware
	install -d release/$(DEVNAME)/firmware/
	@for img in $(ALL_DNGL_IMAGES); do \
	      install -pvD $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).h release/$(DEVNAME)/firmware/$${img}.h; \
	      install -pvD $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin release/$(DEVNAME)/firmware/$${img}.bin; \
	done
	# Install Apps
	install -d release/$(DEVNAME)/apps/$(DEVOS)/$(DEVPROC)
	install -p src/wl/exe/ce/obj/$(BDIR)/wl.exe release/$(DEVNAME)/apps/$(DEVOS)/$(DEVPROC)
	install -p src/wl/exe/ce/obj/server/socket/$(BDIR)/wl_server_socket.exe release/$(DEVNAME)/apps/$(DEVOS)/$(DEVPROC)
	install -p src/wl/exe/ce/obj/server/dongle/$(BDIR)/wl_server_dongle.exe release/$(DEVNAME)/apps/$(DEVOS)/$(DEVPROC)
	install -p src/wl/exe/ce/obj/server/802.11/$(BDIR)/wl_server_wifi.exe release/$(DEVNAME)/apps/$(DEVOS)/$(DEVPROC)
	install -p src/dhd/exe/ce/obj/$(BDIR)/dhd.exe release/$(DEVNAME)/apps/$(DEVOS)/$(DEVPROC)
	# Install Driver
	install -d release/$(DEVNAME)/driver/$(DEVOS)/$(DEVPROC)
	install -p src/dhd/ce/obj/sta/$(BDIR)/bcm$(DEVTYPE).dll release/$(DEVNAME)/driver/$(DEVOS)/$(DEVPROC)
	install -p src/dhd/ce/install/$(DEVNAME)/bcm$(DEVTYPE).reg release/$(DEVNAME)/driver/$(DEVOS)/$(DEVPROC)
	# Install Installer
	install -d release/$(DEVNAME)/installer/$(DEVOS)/$(DEVPROC)
	install -p src/dhd/ce/install/$(DEVNAME)/sta/$(BDIR)/bcm$(DEVTYPE)_ce$(DEVOS)_$(DEVPROC)_setup.exe release/$(DEVNAME)/installer/$(DEVOS)/$(DEVPROC)
	@date +"END:   $@, %D %T"  | tee -a profile.log

# Package Eap Plugins (RAS EAP ttls for now)
$(RLS_EAP_TARGETS): DEVOS  :=500
$(RLS_EAP_TARGETS): DEVPROC=$(word 2,$(subst _,$(space),$(@)))
$(RLS_EAP_TARGETS): DEVNAME:=bcm
$(RLS_EAP_TARGETS): RDIR=$(DEVNAME)/apps/$(DEVOS)/$(DEVPROC)/EAP_Plugins
$(RLS_EAP_TARGETS): checkout mogrify
$(RLS_EAP_TARGETS):
	@date +"START: $@, %D %T"  | tee -a profile.log
	# Install Firmware
	install -d release/$(RDIR)/{Release,Debug}
	# Copy "Release" bits
	install -p src/doc/ReleaseNotesEAPplugins.txt release/$(RDIR)/Release
	$(call COPY_EAP_PLUGINS,RATTLS,$(DEVPROC),Release)
	$(call COPY_EAP_PLUGINS,RASIM,$(DEVPROC),Release)
	$(call COPY_EAP_PLUGINS,RAAKA,$(DEVPROC),Release)
	# Copy "Debug" bits
	install -p src/doc/ReleaseNotesEAPplugins.txt release/$(RDIR)/Debug
	$(call COPY_EAP_PLUGINS,RATTLS,$(DEVPROC),Debug)
	$(call COPY_EAP_PLUGINS,RASIM,$(DEVPROC),Debug)
	$(call COPY_EAP_PLUGINS,RAAKA,$(DEVPROC),Debug)
	@date +"END:   $@, %D %T"  | tee -a profile.log

release: build_dhd pre_release pkg_release
	@date +"START: $@, %D %T"  | tee -a profile.log
	@echo " -- MARK release for external --"
	# stamp all the infs
	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d `date +%m/%d/%Y` "`cygpath -w $(RELEASEDIR)`"
	@date +"END:   $@, %D %T"  | tee -a profile.log

release_src: release $(SRC_TARGETS)

$(SRC_TARGETS): DEVNAME=$(word 2,$(subst _,$(space),$(@)))
$(SRC_TARGETS): DEVPROC=ARM
$(SRC_TARGETS): SDIR=build/$(DEVNAME)
$(SRC_TARGETS): checkout mogrify copy_dongle_images
$(SRC_TARGETS):
	@date +"START: $@, %D %T"  | tee -a profile.log
	install -d build/$(DEVNAME)
	# Copy over release sources to build folder now for verification
	find src components | perl $(SRCFILTER) -v $(SRCFILELIST) | col -b | cpio -p -d $(SDIR)
	# Copy over dongle image for build verification
	@echo "Copying $(ALL_DNGL_IMAGES) to $(SDIR)"
	@for img in $(ALL_DNGL_IMAGES); do \
	     install -d $(SDIR)/$(DNGL_IMGDIR)/$${img}; \
	     install -pv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).h $(SDIR)/$(DNGL_IMGDIR)/$${img}/; \
	     install -pv $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin $(SDIR)/$(DNGL_IMGDIR)/$${img}/; \
	done
	# Temporarily copy over CE SDK libs to validate released sources
	find src/tools/bin/WINCE | grep -v "CVS" | cpio -p -d $(SDIR)
	# Copy over $(DEVNAME) specific CE installer folder
	find src/dhd/ce/install/common | grep -v "CVS" | cpio -p -d $(SDIR)
	find src/dhd/ce/install/$(DEVNAME) | grep -v "CVS" | cpio -p -d $(SDIR)
	# Performing release source build for $(DEVNAME) to validate ....
	$(MAKE) -C $(SDIR)/src/include
	# First validate DHD sources
	$(MAKE) -C $(SDIR)/src/dhd/ce $(if $(WLTEST),WLTEST=1) \
	           TARGETS="$(CUR_BTARGETS)" ce_dhd
	# Next validate DHD installation sources
	gmake   -C $(SDIR)/src/dhd/ce/install $(if $(WLTEST),WLTEST=1) \
	           TARGETS="$(CUR_BTARGETS)" TTYPES="$(DEVTTYPES)"
	# Next validate wl.exe sources
	$(MAKE) -C $(SDIR)/src/wl/exe/ce \
	    $(if $(WLTEST),WLTEST=1) \
	    TARGETS="$(CUR_BTARGETS)" ce_exe
	# Next validate dhd.exe sources
	$(MAKE) -C $(SDIR)/src/dhd/exe/ce \
		$(if $(WLTEST),WLTEST=1) \
		TARGETS="$(CUR_BTARGETS)" ce_exe
	# When filelist contents grow, windows utilities complain that
	# command line is too long. So pass filelist via input file
	cp $(SRCFILELIST) $(SDIR)/
	cd $(SDIR) && find src components | perl $(SRCFILTER) -v $(SRCFILELIST) | \
		grep -v "\.obj\|\.pdb\|\.map\|\.exe\|/CVS" > pkg_these
	echo $(DNGL_IMGDIR)/$(EMBED_DONGLE_IMAGE)/$(DNGL_IMG_PFX).h \
		>> $(SDIR)/pkg_these
	echo src/dhd/ce/install/$(DEVNAME) >> $(SDIR)/pkg_these
	# Copy over validated sources to release/<oem>/src folder now
	tar cpf - -C $(SDIR) -T $(SDIR)/pkg_these \
	    --exclude=*.obj --exclude=*.pdb --exclude=*.map --exclude=*.exe | \
	    tar xvpf - -C release/$(DEVNAME)
	#rm -f $(SDIR)/pkg_these
	# CE redist libs can not be released to src customers. Release MSIs
	rm -rf release/$(DEVNAME)/src/tools/bin/WINCE
	rm -rf $(SDIR)/src/dhd/ce/install/$(DEVNAME)/sta
	rm -rf release/$(DEVNAME)/src/dhd/ce/install/$(DEVNAME)/sta
	install -d release/$(DEVNAME)/src/tools/bin/WINCE
	@install -pv $(foreach devos,$(DEVOSVERS_$(DEVNAME)),\
		     $(foreach devproc,$(DEVPROCS_$(DEVNAME)),\
		     $(MSI_ce$(devos)_$(devproc)))) \
		     release/$(DEVNAME)/src/tools/bin/WINCE
	cd release; $(TREECMD) $(DEVNAME) > Package_Contents.txt
	mv release/Package_Contents.txt release/$(DEVNAME)
	# Package src, firmware, host, apps and installer and docs
	tar cpf release/$(DEVNAME)/dongle-host-driver-source.tar \
	    -C release/$(DEVNAME) src firmware driver apps installer
	tar upf release/$(DEVNAME)/dongle-host-driver-source.tar \
	    -C release/$(DEVNAME) BCMLogo.gif
	gzip -f -9 release/$(DEVNAME)/dongle-host-driver-source.tar
	rm -rf release/$(DEVNAME)/src
	rm -rf $(SDIR)
	@echo "==================================================="
	@date +"END:   $@, %D %T"  | tee -a profile.log

build_clean: release
	-@rmdir -v build
	-@rm -fv $(HNDRTE_FLAG)
	-@find src components -type f -name "*.obj" -o -name "*.OBJ" -o -name "*.o" | \
		xargs rm -f

build_end: build_clean
	@date +"BUILD_END: $(BRAND) TAG=${TAG} %D %T" | tee -a profile.log

#pragma runlocal
%.mk:
	cvs -Q co $(if $(TAG),-r $(TAG)) -p src/tools/release/$@ > $@
ifneq ($(OVERRIDE),)
	-[ -f "$(OVFILE)" ] && cp $(OVFILE) $@
endif

.PHONY: FORCE checkout mogrify build_include build_end copy_dongle_images release release_src $(ALL_BTARGETS) $(ALL_RTARGETS) $(BUILD_TARGETS) $(RLS_TARGETS) $(SRC_TARGETS)
