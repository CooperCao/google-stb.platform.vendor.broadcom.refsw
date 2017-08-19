#
# Common makefile to build BRIX x86 wpa_supplicant on Ubuntu server.
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#

BUILD_BASE         := $(CURDIR)
SRCDIR             := $(BUILD_BASE)/src
BUILDDIR           := $(BUILD_BASE)/build
RELEASEDIR         := $(BUILD_BASE)/release
SVN_BASE           := http://svn.sj.broadcom.com/svn/wlansvn
MOGRIFY_PL         ?= $(SVN_BASE)/proj/trunk/src/tools/build/mogrify.pl
MAKEJOBS           ?= -j 12 -l 48
CHECKOUTJOBS       ?= -j 12
WPA_BASE_DIR	   ?= $(SVN_BASE)/components/opensource/hostap/trunk
WPA_SUPP_DIR	   ?= $(notdir $(WPA_BASE_DIR))
ANDROID_FRAMEWORK  ?= x86-kk-local
FRAMEWORK_MANIFEST ?= http://hnd-swgit.sj.broadcom.com:8080/android/x86-kk-local/platform_manifest
FRAMEWORK_BRANCH   ?=
FRAMEWORK_EXTRA_FLAGS ?=
SHELL              := /bin/bash
OEM_LIST           := base
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE
UNIQ_STR           := $(strip $(shell mktemp -u XXXXXXXX))
INPROGRESS_DIR     := /tmp/wlanswbuild/in-progress
INPROGRESS_UNIQ    := $(INPROGRESS_DIR)/$(UNIQ_STR)
UNIQ_FRAMEWORK     := $(ANDROID_FRAMEWORK)_$(UNIQ_STR)

# handle tagged build
ifneq ($(findstring HOSTAP,$(TAG)),)
  WPA_SUPP_DIR := $(strip $(TAG))
  WPA_BASE_DIR := $(SVN_BASE)/components/opensource/hostap/tags/HOSTAP/$(WPA_SUPP_DIR)
  MOGRIFY_PL   := $(SVN_BASE)/proj/tags/HOSTAP/$(WPA_SUPP_DIR)/src/tools/build/mogrify.pl
endif

# setting framework target (build-buildtype) and framework product name
# if any of the two macros is not passed in from outside
ifndef FRAMEWORK_TARGET
  FRAMEWORK_TARGET   := haswell_generic-eng
endif
ifndef FRAMEWORK_PRODUCT
  FRAMEWORK_PRODUCT  := haswell_generic
endif
ifndef FRAMEWORK_MAKE_IMAGE
  FRAMEWORK_MAKE_IMAGE  :=
endif

FRAMEWORK_OUTDIR   := out/target/product/$(strip $(FRAMEWORK_PRODUCT))

UNIQ_WPA_SUPP_DIR := $(WPA_SUPP_DIR)_$(UNIQ_STR)

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

VALID_BRANDS := ubuntu-external-wpa-supp-test

ifeq ($(filter $(VALID_BRANDS),$(BRAND)),)
  $(error ERROR This $(BRAND) build is not a standalone build brand. Valid brands are $(VALID_BRANDS))
endif

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
        touch $1/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
        rm -f $1/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD


all: get_framework checkout mogrify wpa_build clean_framework


get_framework:
	@$(MARKSTART)
	mkdir -p $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK) \
	&& cd $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK) \
	&& repo init -u $(FRAMEWORK_MANIFEST) $(FRAMEWORK_BRANCH) \
	&& repo sync $(CHECKOUTJOBS)
	@$(MARKEND)

checkout: $(foreach oem,$(OEM_LIST),checkout_$(oem))

$(foreach oem,$(OEM_LIST),checkout_$(oem)): BLDDIR=build/$(strip $(patsubst checkout_%, %, $@))
$(foreach oem,$(OEM_LIST),checkout_$(oem)): OEM=$(strip $(patsubst checkout_%, %, $@))
$(foreach oem,$(OEM_LIST),checkout_$(oem)):
	@$(MARKSTART)
	mkdir -p $(BLDDIR) \
	&& cd $(BLDDIR) \
	&& svn export $(MOGRIFY_PL) \
	&& svn export $(WPA_BASE_DIR) \
	&& mv $(WPA_SUPP_DIR) $(OEM)_$(UNIQ_WPA_SUPP_DIR)
	@$(MARKEND)

mogrify: $(foreach oem,$(OEM_LIST),mogrify_$(oem))

$(foreach oem,$(OEM_LIST),mogrify_$(oem)): BLDDIR=build/$(strip $(patsubst mogrify_%, %, $@))
$(foreach oem,$(OEM_LIST),mogrify_$(oem)): OEM=$(strip $(patsubst mogrify_%, %, $@))
$(foreach oem,$(OEM_LIST),mogrify_$(oem)):
	@$(MARKSTART)
	if [ "$(OEM)" == "base-wapi-ccx" ]; then \
	    cd $(BLDDIR) \
	    && cp ./$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant/64bit_all_features.config ./$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant/android.config \
	    && find ./$(OEM)_$(UNIQ_WPA_SUPP_DIR) | xargs perl ./mogrify.pl -DBRCM_FEATURES_COPYRIGHT -DBRCM_CONTRIBUTE_COPYRIGHT -UBCM_LINUX_BUILD -UCONFIG_BRCM_TBD -UICS; \
	fi
	if [ "$(OEM)" == "base-wapi" ]; then \
	    cd $(BLDDIR) \
	    && cp ./$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant/android_wapi.config ./$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant/android.config \
	    && find ./$(OEM)_$(UNIQ_WPA_SUPP_DIR) | xargs perl ./mogrify.pl -UBRCM_VE -UBRCM_CCX -UBRCM_OKC -DBRCM_FEATURES_COPYRIGHT -DBRCM_CONTRIBUTE_COPYRIGHT -UBCM_LINUX_BUILD -UCONFIG_BRCM_WFDS -UCONFIG_WIFI_DISPLAY_NFC_BCM -UCONFIG_P2P_HACK_POST38 -UCONFIG_BRCM_TBD -UICS -UCONFIG_BRCM_HS20; \
	fi
	if [ "$(OEM)" == "base" ]; then \
	    cd $(BLDDIR) \
	    && find ./$(OEM)_$(UNIQ_WPA_SUPP_DIR) | xargs perl ./mogrify.pl -UBRCM_VE -UBRCM_CCX -UWAPI -UBRCM_OKC -DBRCM_CONTRIBUTE_COPYRIGHT -UBRCM_FEATURES_COPYRIGHT -UBCM_LINUX_BUILD -UBCM_SDO -UBCM_MAP_SDCMDS_2_SDOCMDS -UBCM_GENL -UCONFIG_P2P_NFC -UBRCM_DRV_ROAM -UCONFIG_BRCM_WFDS -UCONFIG_WIFI_DISPLAY_NFC_BCM -UBRCM_DEBUG -UCONFIG_P2P_HACK_POST38 -UCONFIG_WPS_NFC_BCM -UCONFIG_BRCM_SD_DYNAMIC_CAPA -UCONFIG_BRCM_BEST_CHANNEL -UCONFIG_IP_ALLOC -UICS -UCONFIG_BRCM_TBD -UCONFIG_BRCM_HS20; \
	fi
	@$(MARKEND)


# checkout local android kernel/framework from git repo to the brand-build
# build area, then copy different features mogrified wpa_supplicant with
# unique folder name to the prebuilt android FRAMEWORK external area and build
# wpa_supplicant there.
# We need to combine build and release step, since we actually build in the
# the prebuild framework external area, we need to move it to release folder
# right after each oem is built, just to ensure each oem build won't pollute
# each other
wpa_build: $(foreach oem,$(OEM_LIST),wpa_build_$(oem))

$(foreach oem,$(OEM_LIST),wpa_build_$(oem)): RELDIR=release/$(strip $(patsubst wpa_build_%, %, $@))
$(foreach oem,$(OEM_LIST),wpa_build_$(oem)): BLDDIR=build/$(strip $(patsubst wpa_build_%, %, $@))
$(foreach oem,$(OEM_LIST),wpa_build_$(oem)): OEM=$(strip $(patsubst wpa_build_%, %, $@))
$(foreach oem,$(OEM_LIST),wpa_build_$(oem)):
	$(call WARN_PARTIAL_BUILD, $(BLDDIR))
	@$(MARKSTART)
	mkdir -p $(RELDIR)/src/$(WPA_SUPP_DIR)
	cp -a $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/* $(RELDIR)/src/$(WPA_SUPP_DIR)
	if [ "$(OEM)" == "base" ]; then \
	    rm -rf $(RELDIR)/src/$(WPA_SUPP_DIR)/wapilib && rm -rf $(RELDIR)/src/$(WPA_SUPP_DIR)/src/wapi ; \
	fi
	mv $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR) $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)
	- rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/wpa_supplicant_8/wpa_supplicant/Android.mk \
	&& rm -rf $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/wpa_supplicant_8
	- rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/hardware/broadcom/wlan/bcmdhd/wpa_supplicant_8_lib/Android.mk \
	&& rm -rf $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/hardware/broadcom/wlan/bcmdhd/wpa_supplicant_8_lib
	- rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/wpa_supplicant_6/wpa_supplicant/Android.mk \
	&& rm -rf $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/wpa_supplicant_6
	- rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/hardware/broadcom/wlan/bcmdhd/config/Android.mk \
	&& rm -rf $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/hardware/broadcom/wlan
	cd $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/hostapd \
	&& ln -sf ../src src \
	&& cd ../wpa_supplicant \
	&& ln -sf ../src src
	cd $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK) \
	&& unset `env | awk -F= ' /^\w/ {print $$1}' | grep -v PATH | xargs` \
	&& . ~/.bash_profile \
	&& cd $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK) \
	&& . build/envsetup.sh \
	&& lunch haswell_generic-eng \
	&& env \
	&& make $(MAKEJOBS) $(FRAMEWORK_MAKE_IMAGE) $(FRAMEWORK_EXTRA_FLAGS)
	cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/libbcmdhd $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/
	if [ "$(OEM)" != "base" ]; then \
	    cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wapilib $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/ ; \
	fi
	cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wapilib $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/
	cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/hostapd $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/
	cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/
	mkdir -p $(RELDIR)/apps
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/live.img $(RELDIR)/apps/live.img
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/system/bin/hostapd $(RELDIR)/apps/hostapd
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/system/bin/hostapd_cli $(RELDIR)/apps/hostapd_cli
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/system/bin/wpa_supplicant $(RELDIR)/apps/wpa_supplicant
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/system/bin/wpa_cli $(RELDIR)/apps/wpa_cli
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/system/lib/libwpa_client.so $(RELDIR)/apps/libwpa_client.so

#comment# clean up the framework build, after live image and binaries are
#comment# copied to release folder, get ready for the next feature build
	cd $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK) \
	&& $(MAKE) clean
#comment# remove wpa supplicant code, otherwise it will conflict with the next
#comment# wpa supplicant build of next feature
	rm -rf $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)
	cd $(RELDIR)/src \
	&& tar -zcvf $(WPA_SUPP_DIR)_src.tar.gz ./$(WPA_SUPP_DIR) \
	&& mv $(WPA_SUPP_DIR)_src.tar.gz ../
	$(call REMOVE_WARN_PARTIAL_BUILD, $(BLDDIR))
	@$(MARKEND)


clean_framework:
	@$(MARKSTART)
#comment# do a rm test first to make sure we keep rm -rf under control
	rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/Makefile \
	&& rm -rf $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)
	@$(MARKEND)

clean:
	@$(MARKSTART)
	rm -r ./build/
	rm -r ./release/
	@$(MARKEND)

.PHONY: FORCE get_framework checkout mogrify wpa_build clean_framework all $(foreach oem,$(OEM_LIST),checkout_$(oem)) $(foreach oem,$(OEM_LIST),mogrify_$(oem)) $(foreach oem,$(OEM_LIST),wpa_build_$(oem))
