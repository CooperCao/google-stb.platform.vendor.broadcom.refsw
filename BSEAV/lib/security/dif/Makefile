#############################################################################
#    (c)2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
#
#############################################################################
URSR_TOP ?= ../../../..
NEXUS_TOP := $(URSR_TOP)/nexus
BSEAV_TOP := $(URSR_TOP)/BSEAV/
APPLIBS_TOP := $(URSR_TOP)/AppLibs

NXCLIENT_SUPPORT ?= y

# Build the name of this makefile, relative to B_REFSW_TOP (used for recipe tracing)
# Keep this line before any includes!
B_THIS_MAKEFILE_NAME := $(subst $(abspath ${NEXUS_TOP}/..),,$(abspath $(lastword $(MAKEFILE_LIST))))

# variables for out of source builds
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= $(abspath ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR})

# Include nexus/applibs definitions
CHECK_APPLIBS = $(shell ls $(APPLIBS_TOP)/common/common.inc 2> /dev/null)
ifeq ($(CHECK_APPLIBS),)
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc
else
include $(APPLIBS_TOP)/common/common.inc
endif

ifeq ($(NXCLIENT_SUPPORT),y)
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
endif

SECURITY_TOP := $(BSEAV_TOP)/lib/security
COMMON_DRM_TOP := $(SECURITY_TOP)/common_drm

include $(COMMON_DRM_TOP)/lib/prdy_libdir.inc
include $(COMMON_DRM_TOP)/common_drm.inc

include $(SECURITY_TOP)/third_party/widevine/CENC21/oemcrypto_libdir.inc

WIDEVINE_CENC_DIR := $(SECURITY_TOP)/third_party/widevine/CENC21
WVCDM_PREBUILT :=$(WIDEVINE_CENC_DIR)/cdm/$(WVCDM_LIBDIR)
OEMCRYPTO_PREBUILT :=$(WIDEVINE_CENC_DIR)/oemcrypto/$(OEMCRYPTO_LIBDIR)

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
.PHONY : all
all: prep_wvcdm
	$(Q_)$(MAKE) -f dif.mk install

.PHONY : dif_only
dif_only:
	$(Q_)$(MAKE) -f dif.mk install

.PHONY : clean
clean:
	$(Q_)$(MAKE) -f dif.mk clean
	$(Q_)$(MAKE) -C ./examples clean

############################################################################
#                              EXAMPLES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
.PHONY : examples
examples: all
	$(Q_)$(MAKE) -C ./examples


############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
prep_wvcdm: prep_security
	$(Q_)echo [DIF..... prep_wvcdm target];
	$(Q_)echo [DIF..... building protobuf];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/protobuf all > /dev/null
	$(Q_)echo [DIF..... copying protobuf lib...];
	$(Q_)install -c -m 0755 $(B_REFSW_OBJ_ROOT)/BSEAV/lib/protobuf/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/protobuf-2.5.0/$(NEXUS_PLATFORM)/lib/*.so* $(NEXUS_BIN_DIR);
	$(Q_)echo [DIF..... copying gtest lib...];
	$(Q_)install -c -m 0755 $(B_REFSW_OBJ_ROOT)/BSEAV/lib/protobuf/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/protobuf-2.5.0/gtest/lib/.libs/*.so* $(NEXUS_BIN_DIR);
	$(Q_)echo [DIF..... building curl];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/curl all > /dev/null
	$(Q_)echo [DIF..... copying curl];
	$(Q_)install -c -m 0755 $(B_REFSW_OBJ_ROOT)/BSEAV/lib/curl/$(B_REFSW_ARCH).$(DEBUG_SUFFIX)/curl-7.41.0/lib/.libs/*.so* $(NEXUS_BIN_DIR)/;
	$(Q_)echo [DIF..... copying prebuilt oemcrypto and wvcdm];
	$(Q_)install -c -m 0755 $(WVCDM_PREBUILT)/lib*.so $(NEXUS_BIN_DIR)
	$(Q_)install -c -m 0755 $(OEMCRYPTO_PREBUILT)/lib*.so $(NEXUS_BIN_DIR)

prep_nexus:
	$(Q_)echo [DIF..... prep_nexus target ]
ifeq ($(NXCLIENT_SUPPORT),y)
	$(Q_)if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/nxserver ]; then \
		echo [DIF..... building nxclient .... ]; \
		$(MAKE) -C $(NEXUS_TOP)/nxclient >/dev/null; \
	fi
else
	if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/libnexus.so ]; then \
		echo [DIF..... building nexus .... ]; \
		$(MAKE) -C $(NEXUS_TOP)/build >/dev/null; \
	fi
endif

prep_security: prep_nexus
	$(Q_)echo [DIF..... prep_security target ];
	$(Q_)echo [DIF..... building bcrypt];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install >/dev/null;
	$(Q_)echo [DIF..... building cmndrm];
	$(Q_)$(MAKE) -C $(COMMON_DRM_TOP) install > /dev/null
	$(Q_)echo [DIF..... building cmndrmprdy];
	$(Q_)$(MAKE) -C $(COMMON_DRM_TOP) playready > /dev/null
	$(Q_)echo [DIF..... copying Common DRM libs...]
	$(Q_)install -c -m 0755 $(COMMON_DRM_TOP)/lib/$(CMNDRM_DIR)/libcmndrm*.so $(NEXUS_BIN_DIR)
	$(Q_)install -c -m 0755 $(COMMON_DRM_TOP)/$(CMNDRM_PRDY_LIBDIR)/libcmndrm*.so $(NEXUS_BIN_DIR)
ifeq ($(SAGE_SUPPORT),y)
	$(Q_)echo [DIF..... copying sage version Common DRM lib version...]
	$(Q_)install -c -m 0755 $(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/common_drm/lib/$(CMNDRM_DIR)/libcmndrm*.so $(NEXUS_BIN_DIR)
	$(Q_)install -c -m 0755 ${BSEAV_TOP}/lib/playready/${MSDRM_PRDY_SDK_VERSION}/bin/$(ARCH_DIR_PRDY)/lib/libplayreadypk_host.so $(NEXUS_BIN_DIR)
else
	$(Q_)install -c -m 0755 ${BSEAV_TOP}/lib/playready/${MSDRM_PRDY_SDK_VERSION}/bin/$(ARCH_DIR_PRDY)/lib/libplayreadypk.so $(NEXUS_BIN_DIR)
endif
