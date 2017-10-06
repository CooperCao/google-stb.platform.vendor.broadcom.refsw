#
# Common makefile to build windows sdio DHD drivers,
# app, tools, installer and package them
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

# HOW THIS MAKEFILE WORKS (This makefile can NOT be used by itself)
#
# 1. Checkout source from SVN to src/. The directories and modules to
# check out are in $(HNDSVN_BOM).
#
# 2. Run the transmogrifier on all source code to remove some selected
# defines and to force the expression of others. Also some comments
# undefined by the transmogrifier are in $(UNDEFS). The symbols to be
# defined by the transmogrifier are in $(DEFS).
#
# 3. Build binaries include both trapapp and driver
#
# 4. copy binary files to release folder
#

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

## Caller makefile defines BRAND, BUILD_TYPES variables
DATE               := $(shell date -I)
BUILD_BASE         := $(shell pwd)
RELEASEDIR         := $(BUILD_BASE)/release

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

MOGRIFY             = perl src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))
SHELL              := bash.exe
MAKE_MODE          := unix
NULL               := /dev/null
OVFILE              = $(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)
HNDRTE_DEPS        := checkout
HNDRTE_IMGFN       := rtecdc.h
SRCFILELIST        := src/tools/release/win-dhd-filelist.txt
SRCFILTER          := src/tools/build/srcfilter.pl

OVFILE              = $(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)

ALL_DNGL_IMAGES    ?= $(EMBED_DONGLE_IMAGE) $(SDIO_DONGLE_IMAGES)

SRCFILELISTS_COMPONENTS+= src/tools/release/components/wlphy-filelist.txt
SRCFILELIST_ORG    := src/tools/release/win-dhd-filelist.txt
SRCFILELIST        := win-dhd-filelist
SRCFILTER          := src/tools/build/srcfilter.pl

# Win XP dhd drivers
ifeq ($(strip $(BRAND)),win_internal_dongle_sdio)
	XP_x86_DHDDRIVER   := src/dhd/wdm/obj/i386/checked/bcmsddhd.sys
	XP_x64_DHDDRIVER   := src/dhd/wdm/obj/i386/checked/bcmsddhd.sys
else
	XP_x86_DHDDRIVER   := src/dhd/wdm/obj/i386/free/bcmsddhd.sys
	XP_x64_DHDDRIVER   := src/dhd/wdm/obj/i386/free/bcmsddhd.sys
endif # BRAND

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
RELNUM=$(maj).$(min).$(rcnum).$(incremental)

# These are module names and directories that will be checked out of CVS.
HNDSVN_BOM     := hndrte.sparse

# These symbols will be UNDEFINED in the source code by the transmogirifier
#
UNDEFS ?= CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV BCMCCX BCMEXTCCX  \
          BCMSDIODEV WLFIPS BCMP2P BCMSDIONP \
          DEBUG_LOST_INTERRUPTS BCMQT BCMSLTGT DHD_SPROM

# These symbols will be DEFINED in the source code by the transmogirifier
#
DEFS ?= BCM47XX BCM47XX_CHOPS BCMSDIO BCMDHD WIN_DHD

# for file list pre-processing
FILESDEFS = $(DEFS)
FILESUNDEFS =$(patsubst %,NO_%,$(UNDEFS))
FILESDEFS += FILESUNDEFS
GCCFILESDEFS = $(patsubst %,-D%_SRC,$(FILESDEFS))
# use only base includes
GCCFILESDEFS += -DWLPHY_INC

# These are extensions of source files that should be transmogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

define CREATE_DBUS_SDIO_XP_INF
	@echo "#- $0"
	@echo "#- Generating `basename $2` from `basename $1`"
	@cmd /c type $(subst /,\\,$1) | grep -n "\[strings\..*\]" | cut -d: -f1 | head -1 > lcnumxp.txt 2> $(NULL)
	@lcnum=`cat lcnumxp.txt`; \
	lcnum=`expr $$lcnum - 1`; \
	echo "Extracing first $$lcnum from $1"; \
	cmd /c type $(subst /,\\,$1) 2> $(NULL) | \
		head -$$lcnum | \
		sed -e 's/bcmsddhd.sys/bcmsdstddhdxp.sys/gi' \
			-e 's/bcmsddhd64.sys/bcmsdstddhdxp64.sys/gi' > $2
	@echo "`basename $2` is created from `basename $1` successfully"
	@rm -f lcnumxp.txt
endef # CREATE_DBUS_SDIO_XP_INF

all: build_start checkout mogrify copy_dongle_images build_driver release post_release build_end

include linux-dongle-image-launch.mk
include unreleased-chiplist.mk
include $(MOGRIFY_RULES)
include $(BRAND_RULES)

# check out files
checkout : $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

filelists :
	@$(MARKSTART)
	# Temporary filelist generation for SVN build test
	cat $(SRCFILELIST_ORG) $(SRCFILELISTS_COMPONENTS) | \
		src/tools/release/rem_comments.awk > master_filelist.h
	gcc -E -undef $(GCCFILESDEFS) -Ulinux -o $(SRCFILELIST) master_filelist.h
	@$(MARKEND)	

define MOGRIFY_LIST
	/usr/bin/find src components $(MOGRIFY_EXCLUDE) -type f -print |  \
		perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' > \
		src/.mogrified
endef

build_include: checkout filelists mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)

# TODO: Convert this dhd build similar to linux-dhd.mk
build_driver: checkout mogrify build_include copy_dongle_images
	@$(MARKSTART)
	@echo " -- MARK build driver $(BLDTYPE) --"
ifeq ($(BCM_MFGTEST),)
	$(MAKE) -C src/dhd/wdm -f sdio.mk
	$(MAKE) -C src/dhd/wdm -f sdspi.mk
#	$(MAKE) -C src/dhd/wdm -f gspi.mk
	$(MAKE) -C src/dhd/wdm EMBED_IMG_NAME=$(DBUS_SDIO_DONGLE_IMAGE) \
		build_sdstd_xp_driver
	$(MAKE) -C src/dhd/wdm EMBED_IMG_NAME=$(DBUS_SDIO_DONGLE_IMAGE) \
		build_sdstd_vista_driver
	$(MAKE) -C src/wl/exe OTHER_SOURCES=""
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll
	$(MAKE) -C src/dhd/exe OTHER_SOURCES=""
else # MFGTEST
	$(MAKE) -C src/dhd/wdm WLTEST=1 -f sdio.mk
	$(MAKE) -C src/dhd/wdm WLTEST=1 -f sdspi.mk
#	$(MAKE) -C src/dhd/wdm WLTEST=1 -f gspi.mk
	$(MAKE) -C src/dhd/wdm EMBED_IMG_NAME=$(DBUS_SDIO_DONGLE_IMAGE) \
		WLTEST=1 build_sdstd_xp_driver
	$(MAKE) -C src/dhd/wdm EMBED_IMG_NAME=$(DBUS_SDIO_DONGLE_IMAGE) \
		WLTEST=1 build_sdstd_vista_driver
	$(MAKE) -C src/wl/exe WLTEST=1 OTHER_SOURCES=""
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll
	$(MAKE) -C src/dhd/exe WLTEST=1 OTHER_SOURCES=""
endif # BCM_MFGTEST
	@$(MARKEND)

clean:
	@$(MARKSTART)
	$(MAKE) -C src/include $@
	$(MAKE) -C src/wl/exe $@
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll $@
	$(MAKE) -C src/dhd/exe $@
	$(MAKE) -C src/dhd/wdm -f sdio.mk $@
	$(MAKE) -C src/dhd/wdm -f sdspi.mk $@
	@$(MARKEND)

## Win XP/2K release packaging after everything is built
release: $(RELEASEDIR)/BcmDHD
	@$(MARKSTART)
	-install src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)
#
# Release the BCM branded files.  These are really 'unbranded'
# by definition, but some customers may get these files and just
# live with that fact that they have Broadcom logos and copyrights.
#
# Win XP/2k
$(RELEASEDIR)/BcmDHD::  $(RELEASEDIR)/BcmDHD/Bcm_Sdio_DriverOnly \
			$(RELEASEDIR)/BcmDHD/Bcm_SdSPI_DriverOnly \
			$(RELEASEDIR)/BcmDHD/Bcm_gSPI_DriverOnly \
			$(RELEASEDIR)/WinXP/BcmDHD/Bcm_DbusSdio_DriverOnly \
			$(RELEASEDIR)/BcmDHD/Bcm_Firmware \
			$(RELEASEDIR)/BcmDHD/Bcm_Apps
	@$(MARKSTART)
	install -p src/doc/BCMLogo.gif $@
	@echo "Updating release number in releasenotes"
	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$@/ReleaseNotes.htm
	@$(MARKEND)

# Win XP/2k
$(RELEASEDIR)/BcmDHD/Bcm_Sdio_DriverOnly:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	install $(XP_x86_DHDDRIVER) $@/
	install $(XP_x64_DHDDRIVER) $@/bcmsddhd64.sys
ifeq ($(strip $(BRAND)),win_internal_dongle_sdio)
	install $(subst .sys,.pdb,$(XP_x86_DHDDRIVER)) $@/
#disabled## Both 32bit and 64bit share the same sys and pdb files
#disabled#install $(subst .sys,.pdb,$(XP_x64_DHDDRIVER)) $@/
endif # BRAND
	install -p src/dhd/wdm/bcmsddhd.inf $@/
	install -p src/doc/BCMLogo.gif $@
	@echo "Updating release number in releasenotes"
	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$@/ReleaseNotes.htm
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE

$(RELEASEDIR)/BcmDHD/Bcm_SdSPI_DriverOnly:: FORCE
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	# install driver
ifeq ($(strip $(BRAND)),win_internal_dongle_sdio)
	install -p src/dhd/wdm/obj-sdspi/i386/checked/bcmsdspidhd.sys $@/
	install -p src/dhd/wdm/obj-sdspi/i386/checked/bcmsdspidhd.pdb $@/
else
	install -p src/dhd/wdm/obj-sdspi/i386/free/bcmsdspidhd.sys $@/
endif
	install -p src/dhd/wdm/bcmsdspidhd.inf               $@/bcmsdspidhd.inf
	install -p src/doc/BCMLogo.gif $@
	@echo "Updating release number in releasenotes"
	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$@/ReleaseNotes.htm
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

$(RELEASEDIR)/BcmDHD/Bcm_gSPI_DriverOnly:: FORCE
#	mkdir -p $@
#	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
#	# install driver
#ifeq ($(strip $(BRAND)),win_internal_dongle_sdio)
#	install -p src/dhd/wdm/obj-gspi/i386/checked/bcmgspidhd.sys $@/
#else
#	install -p src/dhd/wdm/obj-gspi/i386/free/bcmgspidhd.sys $@/
#endif
#	install -p src/dhd/wdm/bcmgspidhd.inf               $@/bcmgspidhd.inf
#	install -p src/shared/nvram/bcm94329_nokia.txt               $@/noknvram.txt
#	install -p src/doc/BCMLogo.gif $@
#	@echo "Updating release number in releasenotes"
#	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
#		src/doc/ReleaseNotesNokia.html > \
#		$@/ReleaseNotes.htm
#	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE

# WinXP SDIO Dbus driver package
$(RELEASEDIR)/WinXP/BcmDHD/Bcm_DbusSdio_DriverOnly:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	# install driver
ifeq ($(strip $(BRAND)),win_internal_dongle_sdio)
	install -p src/dhd/wdm/buildsdstdxp/objchk_wxp_x86/i386/bcmsdstddhdxp.sys $@/
	install -p src/dhd/wdm/buildsdstdxp/objchk_wxp_x86/i386/bcmsdstddhdxp.pdb $@/
else
	install -p src/dhd/wdm/buildsdstdxp/objfre_wxp_x86/i386/bcmsdstddhdxp.sys $@/
endif # BRAND
#	install -p src/dhd/wdm/bcmsddhd.inf $@/bcmsdstddhdxp.inf
	$(call CREATE_DBUS_SDIO_XP_INF,src/dhd/wdm/bcmsddhd.inf,$@/bcmsdstddhdxp.inf)
	install -p src/doc/BCMLogo.gif $@
	@echo "Updating release number in releasenotes"
	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$@/ReleaseNotes.htm
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)	

# TODO: Currently Vista packaging is not enabled even though it is built
# WinVista SDIO Dbus driver package
$(RELEASEDIR)/WinVista/BcmDHD/Bcm_DbusSdio_DriverOnly:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	# install driver
ifeq ($(strip $(BRAND)),win_internal_dongle_sdio)
	install -p src/dhd/wdm/buildsdstdxp/objchk_wlh_x86/i386/bcmsdstddhdlh.sys $@/
	install -p src/dhd/wdm/buildsdstdxp/objchk_wlh_x86/i386/bcmsdstddhdlh.pdb $@/
else
	install -p src/dhd/wdm/buildsdstdxp/objfre_wlh_x86/i386/bcmsdstddhdlh.sys $@/
endif # BRAND
#	install -p src/dhd/wdm/bcmsddhd.inf $@/bcmsdstddhdlh.inf
	$(call CREATE_DBUS_SDIO_VISTA_INF,src/dhd/wdm/bcmsddhd.inf,$@/bcmsdstddhdlh.inf)
	install -p src/doc/BCMLogo.gif $@
	@echo "Updating release number in releasenotes"
	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$@/ReleaseNotes.htm
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

# Win XP/2k
$(RELEASEDIR)/BcmDHD/Bcm_Apps:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
ifeq ($(strip $(BRAND)),win_internal_dongle_sdio)
	install -p src/wl/exe/windows/winxp/obj/checked/wl.exe $@/
	install -p src/wl/exe/windows/win7/obj/wlm/checked/wlm.lib $@/
	install -p src/wl/exe/windows/win7/obj/wlm/checked/wlm.dll $@/
	install -p src/dhd/exe/windows/winxp/obj/checked/dhd.exe $@/
else
	install -p src/wl/exe/windows/winxp/obj/free/wl.exe $@/
	install -p src/wl/exe/windows/win7/obj/wlm/free/wlm.lib $@/
	install -p src/wl/exe/windows/win7/obj/wlm/free/wlm.dll $@/
	install -p src/dhd/exe/windows/winxp/obj/free/dhd.exe $@/
endif
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE

$(RELEASEDIR)/BcmDHD/Bcm_Firmware:: FORCE
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@for img in $(ALL_DNGL_IMAGES); do \
		install -pvD $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).h $@/$${img}.h; \
		install -pvD $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin $@/$${img}.bin; \
	done
	# install nvram
ifdef SDIO_NVRAM_FILES
	install -p $(SDIO_NVRAM_FILES:%=src/shared/nvram/%) $@/
endif # SDIO_NVRAM_FILES
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

# TODO: Convert this source verification similar to linux-dhd.mk
# NOTE: We no longer need to release source bits for winxp as discussed
# NOTE: with esta s/w managers. Following target is not called by anyone
$(RELEASEDIR)/BcmDHD/Bcm_Src:
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	cp $(SRCFILELIST) $(SRCFILELIST)_bcm
	perl src/tools/build/mogrify.pl $(DEFS_bcm) $(SRCFILELIST)_bcm
	tar cpf - $(TAR_SKIPCMD) `find src components $(FIND_SKIPCMD) -print | perl $(SRCFILTER) -v $(SRCFILELIST)_bcm` \
	          `find build/dongle/* -type f \
			-name "*$(DNGL_IMG_PFX).h" -o -name "*$(DNGL_IMG_PFX).bin"` | \
		   tar xpf - $(TAR_SKIPCMD) -C $@
	$(MAKE) -C $@/src/include
	$(MAKE) -C $@/src/dhd/wdm -f sdio.mk
	$(MAKE) -C $@/dhd/exe OTHER_SOURCES=""
	$(MAKE) -C $@/src/wl/exe OTHER_SOURCES=""
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll
	$(MAKE) -C $@/src/dhd/wdm -f sdio.mk clean
	$(MAKE) -C $@/dhd/exe OTHER_SOURCES="" clean
	$(MAKE) -C $@/src/wl/exe OTHER_SOURCES="" clean
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll clean
	rm -rf $@/
	mkdir -p $@
	tar cpf - $(TAR_SKIPCMD) `find src components $(FIND_SKIPCMD) -print | perl $(SRCFILTER) -v $(SRCFILELIST)_bcm` \
	          `find build/dongle/* -type f \
			-name "*$(DNGL_IMG_PFX).h" -o -name "*$(DNGL_IMG_PFX).bin"` | \
		   tar xpf - $(TAR_SKIPCMD) -C $@
	rm -f $(SRCFILELIST)_bcm
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

post_release: release
	@$(MARKSTART)
	@echo " -- MARK release for external --"
	# stamp all the infs under $(RELEASEDIR)
	# or specify multiple $(RELEASEDIR)/<OEM> folders optionally
	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d `date +%m/%d/%Y` "`cygpath -w $(RELEASEDIR)`"
	@$(MARKEND)

#
# Convenience pseudo-targets
#

# Run mogrifier
mogrify: checkout
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

#pragma runlocal
build_clean: release
	@$(MARKSTART)
	-@find src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" -print | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

build_end: build_clean
	rm -f $(HNDRTE_FLAG)
	@$(MARKEND_BRAND)

.PHONY: release build_driver copy_dongle_images checkout mogrify release build_include build_end FORCE
