#
# Common makefile to build wpa_supplicant on Ubuntu server.
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
WPA_BASE_DIR	   ?= $(SVN_BASE)/components/opensource/hostap/trunk
WPA_SUPP_DIR	   ?= $(notdir $(WPA_BASE_DIR))
ANDROID_FRAMEWORK  ?= android-4.2.1_r1_prebuild
ANDROID_FRAMEWORK_TGZ := $(strip $(ANDROID_FRAMEWORK)).tgz
SHELL              := /bin/bash
OEM_LIST           := base base-wapi base-wapi-ccx
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE
UNIQ_STR           := $(strip $(shell mktemp -u XXXXXXXX))
FRAMEWORK_DIR      := /projects/hnd_swgit/android/android_framework_production
INPROGRESS_DIR     := $(CURDIR)/BLDTMP/in-progress
INPROGRESS_UNIQ    := $(INPROGRESS_DIR)/$(UNIQ_STR)
UNIQ_FRAMEWORK     := $(ANDROID_FRAMEWORK)_$(UNIQ_STR)

# handle tagged build
ifneq ($(findstring HOSTAP,$(TAG)),)
  WPA_SUPP_DIR := $(strip $(TAG))
  WPA_BASE_DIR := $(SVN_BASE)/components/opensource/hostap/tags/HOSTAP/$(WPA_SUPP_DIR)
  MOGRIFY_PL   := $(SVN_BASE)/proj/tags/HOSTAP/$(WPA_SUPP_DIR)/src/tools/build/mogrify.pl
endif

# define the frameworks currently supported by this brand build
VALID_FRAMEWORK := android-4.1.1_r1_prebuild android-4.2.1_r1_prebuild

# exit if the pass in android framework version is not supported
ifeq ($(filter $(VALID_FRAMEWORK),$(ANDROID_FRAMEWORK)),)
  $(error ERROR This framework $(ANDROID_FRAMEWORK) is not currently supported. Valid frameworks are $(VALID_FRAMEWORK))
endif

# setting framework target (build-buildtype) and framework product name
# if any of the two macros is not passed in from outside
ifneq ($(findstring android-4.1.1_r1,$(ANDROID_FRAMEWORK)),)
ifndef FRAMEWORK_TARGET
  FRAMEWORK_TARGET   := full_maguro-userdebug
endif
ifndef FRAMEWORK_PRODUCT
  FRAMEWORK_PRODUCT  := maguro
endif
else  # else here means 4.2.1_r1, need to update the else section if changed
ifndef FRAMEWORK_TARGET
  FRAMEWORK_TARGET   := full_manta-userdebug
endif
ifndef FRAMEWORK_PRODUCT
  FRAMEWORK_PRODUCT  := manta
endif
endif
FRAMEWORK_OUTDIR   := out/target/product/$(strip $(FRAMEWORK_PRODUCT))/system

UNIQ_WPA_SUPP_DIR := $(WPA_SUPP_DIR)_$(UNIQ_STR)

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

VALID_BRANDS := ubuntu-external-wpa-supp

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


all: copy_framework checkout mogrify wpa_build clean_framework


copy_framework:
	@$(MARKSTART)
	mkdir -p -v $(INPROGRESS_UNIQ) \
	&& cd $(INPROGRESS_UNIQ) \
	&& tar -zxf $(FRAMEWORK_DIR)/$(ANDROID_FRAMEWORK_TGZ) \
	&& mv $(INPROGRESS_UNIQ)/$(ANDROID_FRAMEWORK) $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK) \
	&& rmdir $(INPROGRESS_UNIQ)
	@$(MARKEND)

checkout: $(foreach oem,$(OEM_LIST),checkout_$(oem))

$(foreach oem,$(OEM_LIST),checkout_$(oem)): BLDDIR=build/$(strip $(patsubst checkout_%, %, $@))
$(foreach oem,$(OEM_LIST),checkout_$(oem)): OEM=$(strip $(patsubst checkout_%, %, $@))
$(foreach oem,$(OEM_LIST),checkout_$(oem)):
	@$(MARKSTART)
	mkdir -p -v $(BLDDIR) \
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
	if [ "$(OEM)" == "base" ]; then \
	    cd $(BLDDIR) \
	    && find ./$(OEM)_$(UNIQ_WPA_SUPP_DIR) | xargs perl ./mogrify.pl -UBRCM_VE -UBRCM_CCX -UWAPI -UBRCM_OKC -DBRCM_CONTRIBUTE_COPYRIGHT -UBRCM_FEATURES_COPYRIGHT -UBCM_LINUX_BUILD -UBCM_SDO -UBCM_MAP_SDCMDS_2_SDOCMDS -UBCM_GENL -UCONFIG_P2P_NFC -UBRCM_DRV_ROAM -UCONFIG_BRCM_WFDS -UCONFIG_WIFI_DISPLAY_NFC_BCM -UBRCM_DEBUG -UCONFIG_P2P_HACK_POST38 -UCONFIG_WPS_NFC_BCM -UCONFIG_BRCM_SD_DYNAMIC_CAPA -UCONFIG_BRCM_BEST_CHANNEL -UCONFIG_IP_ALLOC -UICS -UCONFIG_BRCM_TBD -UCONFIG_BRCM_HS20; \
	fi
	if [ "$(OEM)" == "base-wapi" ]; then \
	    cd $(BLDDIR) \
	    && cp ./$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant/android_wapi.config ./$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant/android.config \
	    && find ./$(OEM)_$(UNIQ_WPA_SUPP_DIR) | xargs perl ./mogrify.pl -UBRCM_VE -UBRCM_CCX -UBRCM_OKC -DBRCM_FEATURES_COPYRIGHT -DBRCM_CONTRIBUTE_COPYRIGHT -UBCM_LINUX_BUILD -UCONFIG_BRCM_WFDS -UCONFIG_WIFI_DISPLAY_NFC_BCM -UCONFIG_P2P_HACK_POST38 -UCONFIG_BRCM_TBD -UICS -UCONFIG_BRCM_HS20; \
	fi
	if [ "$(OEM)" == "base-wapi-ccx" ]; then \
	    cd $(BLDDIR) \
	    && cp ./$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant/android_all_features.config ./$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant/android.config \
	    && find ./$(OEM)_$(UNIQ_WPA_SUPP_DIR) | xargs perl ./mogrify.pl -DBRCM_FEATURES_COPYRIGHT -DBRCM_CONTRIBUTE_COPYRIGHT -UBCM_LINUX_BUILD -UCONFIG_BRCM_TBD -UICS; \
	fi
	@$(MARKEND)

# Copy the prebuilt android kernel/framework over to the brand-build build
# area, then copy different features mogrified wpa_supplicant with
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
	mkdir -p -v $(RELDIR)/src/$(WPA_SUPP_DIR)
	cp -a $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/* $(RELDIR)/src/$(WPA_SUPP_DIR)
	if [ "$(OEM)" == "base" ]; then \
	    rm -rf $(RELDIR)/src/$(WPA_SUPP_DIR)/wapilib && rm -rf $(RELDIR)/src/$(WPA_SUPP_DIR)/src/wapi ; \
	fi
	mv $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR) $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)
	cd $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK) \
	&& . build/envsetup.sh \
	&& lunch $(FRAMEWORK_TARGET) \
	&& cd external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/libbcmdhd \
	&& mm -B  && \
	if [ "$(OEM)" != "base" ]; then \
	    cd $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wapilib && mm -B ; \
	fi ; \
	cd $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/hostapd \
	&& ln -sf ../src src \
	&& mm -B \
	&& cd ../wpa_supplicant \
	&& ln -sf ../src src \
	&& mm -B
	cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/libbcmdhd $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/
	if [ "$(OEM)" != "base" ]; then \
	    cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wapilib $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/ ; \
	fi
	cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/hostapd $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/
	cp -a $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/external/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/wpa_supplicant $(BLDDIR)/$(OEM)_$(UNIQ_WPA_SUPP_DIR)/
	mkdir -p -v $(RELDIR)/apps
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/bin/hostapd $(RELDIR)/apps/hostapd
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/bin/hostapd_cli $(RELDIR)/apps/hostapd_cli
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/bin/wpa_supplicant $(RELDIR)/apps/wpa_supplicant
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/bin/wpa_cli $(RELDIR)/apps/wpa_cli
	cp $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/lib/libwpa_client.so $(RELDIR)/apps/libwpa_client.so
	rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/bin/hostapd
	rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/bin/hostapd_cli
	rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/bin/wpa_supplicant
	rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/bin/wpa_cli
	rm $(INPROGRESS_DIR)/$(UNIQ_FRAMEWORK)/$(FRAMEWORK_OUTDIR)/lib/libwpa_client.so
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

.PHONY: FORCE copy_framework checkout mogrify wpa_build clean_framework all $(foreach oem,$(OEM_LIST),checkout_$(oem)) $(foreach oem,$(OEM_LIST),mogrify_$(oem)) $(foreach oem,$(OEM_LIST),wpa_build_$(oem))
