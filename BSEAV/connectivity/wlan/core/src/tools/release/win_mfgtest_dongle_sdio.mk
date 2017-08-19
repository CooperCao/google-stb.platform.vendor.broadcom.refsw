#
# Build Win XP MFGTEST DHD and embed mfgtest SDIO Dongle and package
#
# TODO: This build brand need to change to use win_dhd.mk or
# win_external_dongle_sdio (similar to corresponding linux build brands)
#
# $Id$
#

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

PARENT_MAKEFILE :=
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

SHELL                 := bash.exe
NULL                  := /dev/null
BUILD_BASE            := $(shell pwd)
RELEASEDIR            := $(BUILD_BASE)/release
EMBED_DONGLE_IMAGE    ?=
MFGTEST_DONGLE_IMAGES ?=
DBUS_SDIO_DONGLE_IMAGE :=
BRAND                 ?= win_mfgtest_dongle_sdio

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

MOGRIFY                = perl src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))

## Include any other dongle images needed
SDIO_DONGLE_IMAGES    := \
	$(EMBED_DONGLE_IMAGE) \
	$(MFGTEST_DONGLE_IMAGES) \
	$(DBUS_SDIO_DONGLE_IMAGE)

ALL_DNGL_IMAGES       := $(SDIO_DONGLE_IMAGES)

HNDRTE_IMGFN          := rtecdc.h

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

HNDSVN_BOM   := hndrte_MFGTEST.sparse

# These symbols will be UNDEFINED in the source code by the transmogirifier
UNDEFS += CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV BCMCCX BCMEXTCCX  \
          BCMSDIODEV BCMINTERNAL WLFIPS DHD_SPROM BCMP2P

# These symbols will be DEFINED in the source code by the transmogirifier
DEFS += BCM47XX BCM47XX_CHOPS BCMSDIO

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

all: build_start mogrify build_include copy_dongle_images build_dhd build_apps release post_release build_end

# include $(PARENT_MAKEFILE)

include linux-dongle-image-launch.mk
include $(MOGRIFY_RULES)
include $(BRAND_RULES)

build_start:
	@$(MARKSTART_BRAND)

define MOGRIFY_LIST
	/usr/bin/find src components $(MOGRIFY_EXCLUDE) -type f -print  |  \
		perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' > \
		src/.mogrified
endef

mogrify:
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

build_include: mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)

build_apps:
	@$(MARKSTART)
	$(MAKE) -C src/wl/exe WLTEST=1 OTHER_SOURCES=""
	$(MAKE) -C src/dhd/exe WLTEST=1 OTHER_SOURCES=""
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll
#	$(MAKE) -C src/tools/mfgc wl_base
#	$(MAKE) -C src/tools/mfgc nokmfg
	$(MAKE) -C src/tools/misc nvserial.exe
	@$(MARKEND)

build_dhd:
	@$(MARKSTART)

	$(MAKE) -C src/dhd/wdm -f sdio.mk WLTEST=1 DNGL_IMAGE_NAME=$(EMBED_DONGLE_IMAGE)
	$(MAKE) -C src/dhd/wdm -f sdio.mk clean

	@$(MARKEND)

pre_release:
	@$(MARKSTART)
	mkdir -p $(RELEASEDIR)/BcmDHD/Bcm_Apps
	mkdir -p $(RELEASEDIR)/BcmDHD/Bcm_Sdio_DriverOnly
	mkdir -p $(RELEASEDIR)/Nokia/App
	mkdir -p $(RELEASEDIR)/WLM
	@$(MARKEND)

## Now all builds are completed, package the release now
## TODO: The release directory structure need to be split into Xp, Vista
## TODO: and acorss different driver types followed by chips
$(RELEASEDIR)/BcmDHD::  $(RELEASEDIR)/BcmDHD/Bcm_Apps \
			$(RELEASEDIR)/BcmDHD/Bcm_Sdio_DriverOnly \
			$(RELEASEDIR)/BcmDHD/Bcm_Firmware
	@$(MARKSTART)
	install -p src/doc/BCMLogo.gif $@/
	@echo "Updating release number in releasenotes"
	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$@/ReleaseNotes.htm
	@$(MARKEND)

$(RELEASEDIR)/Nokia::   $(RELEASEDIR)/Nokia/App

$(RELEASEDIR)/BcmDHD/Bcm_Apps:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	install -p src/wl/exe/windows/winxp/obj/free/wl.exe $@/
	install -p src/dhd/exe/windows/winxp/obj/free/dhd.exe $@/
#	install -p src/wl/exe/windows/winxp/obj/mfg_dll/free/brcm_wlu.dll $@/
#	install -p src/tools/mfgc/release/brcm_wlu.dll $@/
#	install -p src/wl/exe/windows/win7/obj/wlm/free/wlm.lib        $@/
#	install -p src/wl/exe/windows/win7/obj/wlm/free/wlm.dll        $@/
	install -p src/tools/misc/nvserial.exe            $@/
	@$(MARKEND)

$(RELEASEDIR)/BcmDHD/Bcm_Firmware:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	# install firmware
	@for img in $(ALL_DNGL_IMAGES); do \
		install -pvD $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).h $@/$${img}.h; \
		install -pvD $(DNGL_IMGDIR)/$${img}/$(DNGL_IMG_PFX).bin $@/$${img}.bin; \
	done
	# install nvram
ifdef SDIO_NVRAM_FILES
	install -p $(SDIO_NVRAM_FILES:%=src/shared/nvram/%) $@/
endif # SDIO_NVRAM_FILES
	@$(MARKEND)

$(RELEASEDIR)/BcmDHD/Bcm_Sdio_DriverOnly:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	# install driver
	install -p src/dhd/wdm/obj/i386/free/bcmsddhd.sys $@/
	install -p src/dhd/wdm/bcmsddhd-mfg.inf $@/bcmsddhd.inf
	install -p src/doc/BCMLogo.gif $@/
	@echo "Updating release number in releasenotes"
	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$@/ReleaseNotes.htm
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)	

# WinXP SDIO Dbus driver package
$(RELEASEDIR)/WinXP/BcmDHD/Bcm_DbusSdio_DriverOnly:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	# install driver
	install -p src/dhd/wdm/buildsdstdxp/objfre_wxp_x86/i386/bcmsdstddhdxp.sys $@/
	install -p src/dhd/wdm/bcmsddhd.inf $@/bcmsdstddhdxp.inf
	$(call CREATE_DBUS_SDIO_XP_INF,src/dhd/wdm/bcmsddhd.inf,$@/bcmsdstddhdxp.inf)
	install -p src/doc/BCMLogo.gif $@
	@echo "Updating release number in releasenotes"
	@sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesDongleSdio.html > \
		$@/ReleaseNotes.htm
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

$(RELEASEDIR)/Nokia/App:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	install -p src/doc/BCMLogo.gif $@/
	sed -e "s/maj.min.rcnum.incr/$(RELNUM)/g" \
		src/doc/ReleaseNotesNokia.html > $@/ReleaseNotes.htm
#	To generate proper header file for vee setup need this SED ing
#	mkdir -p tmp
#	install -p src/tools/mfgc/nokia/include/vee_wrapper.h  tmp/ignore_me.h
#	sed -e "/EXPORT/s///" -e "/^#/s/^#.*$$//" tmp/ignore_me.h > tmp/vee_wrapper.h
#	install -p tmp/vee_wrapper.h $@/
#	rm -rf tmp
#	install -p src/tools/mfgc/nokia/cli/free/nok_cli.exe $@/
#	install -p src/tools/mfgc/nokia/dll/free/nok_mfg.dll $@/
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

$(RELEASEDIR)/WLM:: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	make -f src/tools/release/win_wlm.mk package
	install -p wlm.tar.gz   $@/
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

post_release:
# stamp all the infs
	@$(MARKSTART)
	@echo maj=$(maj)
	@echo min=$(min)
	@echo rcnum=$(rcnum)
	@echo incremental=$(incremental)
	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d `date +%m/%d/%Y` "`cygpath -w $(RELEASEDIR)`"
	@$(MARKEND)

release: pre_release $(RELEASEDIR)/BcmDHD $(RELEASEDIR)/Nokia $(RELEASEDIR)/WLM post_release

build_clean: release
	@$(MARKSTART)
	-@find src components -type f -name "*.obj" -o -name "*.OBJ" -o -name "*.o" | \
		xargs rm -f
	@$(MARKEND)

build_end: build_clean
	@rm -fv $(HNDRTE_FLAG)
	@$(MARKEND_BRAND)

.PHONY: FORCE mogrify build_include build_dhd build_apps  release pre_release post_release
