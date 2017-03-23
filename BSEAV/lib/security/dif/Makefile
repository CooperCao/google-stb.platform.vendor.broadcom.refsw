############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
############################################################
URSR_TOP ?= ../../../..
NEXUS_TOP := $(URSR_TOP)/nexus
BSEAV_TOP := $(URSR_TOP)/BSEAV/
APPLIBS_TOP := $(URSR_TOP)/AppLibs

NXCLIENT_SUPPORT ?= y

# Redirect logs to
LOG_REDIRECT_FILE ?= /dev/null

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
WIDEVINE_CENC_DIR := $(SECURITY_TOP)/third_party/widevine/CENC21

include $(COMMON_DRM_TOP)/lib/prdy_libdir.inc
include $(COMMON_DRM_TOP)/common_drm.inc
include $(BSEAV_TOP)/lib/playready/3.0/prdy30_libdir.inc

WIDEVINE_SUPPORT ?= y
ifneq ($(shell ls "$(WIDEVINE_CENC_DIR)" 2>/dev/null|grep -m1 -o Makefile), Makefile)
    WIDEVINE_SUPPORT = n
    $(warning !!!!! Widevine is disabled !!!!!)
endif

ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux), ${B_REFSW_ARCH})
PREBLT_LIB_SUFFIX := 64
endif

NJOBS ?= 1

############################################################################
#                              MAIN TARGETS
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
.PHONY : all
all: prep_wvcdm
	$(Q_)export WIDEVINE_SUPPORT=$(WIDEVINE_SUPPORT); \
	$(MAKE) -C ./src install

.PHONY : dif_only
dif_only:
	$(Q_)export WIDEVINE_SUPPORT=$(WIDEVINE_SUPPORT); \
	$(MAKE) -C ./src install

.PHONY : clean
clean: localclean
ifeq ($(WIDEVINE_SUPPORT),y)
	$(Q_)echo [DIF..... cleaning Widevine];
	$(Q_)$(MAKE) -C $(WIDEVINE_CENC_DIR) clean
endif

localclean:
	$(Q_)echo [DIF..... cleaning DIF];
	$(Q_)export WIDEVINE_SUPPORT=$(WIDEVINE_SUPPORT); \
	$(MAKE) -C ./src clean
	$(Q_)$(MAKE) -C ./examples clean

############################################################################
#                              EXAMPLES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
.PHONY : examples
examples: all
	$(Q_)export WIDEVINE_SUPPORT=$(WIDEVINE_SUPPORT); \
	$(MAKE) -C ./examples


############################################################################
#                             BUILD RULES
#vvvv#####################vvvvvvvvvvvvvvvvvvvvvvv#####################vvvv##
prep_wvcdm: prep_security
ifeq ($(WIDEVINE_SUPPORT),y)
	$(Q_)echo [DIF..... building Widevine];
	$(Q_)$(MAKE) -C $(WIDEVINE_CENC_DIR) install > $(LOG_REDIRECT_FILE)
endif

prep_nexus:
	$(Q_)echo [DIF..... prep_nexus target ]
ifeq ($(NXCLIENT_SUPPORT),y)
	$(Q_)if [ ! -e $(NEXUS_BIN_DIR)/nxserver ]; then \
		echo [DIF..... building nxclient .... ]; \
		$(MAKE) -j$(NJOBS) -C $(NEXUS_TOP)/nxclient > $(LOG_REDIRECT_FILE); \
	fi
else
	if [ ! -e $(NEXUS_BIN_DIR)/libnexus.so ]; then \
		echo [DIF..... building nexus .... ]; \
		$(MAKE) -j$(NJOBS) -C $(NEXUS_TOP)/build > $(LOG_REDIRECT_FILE); \
	fi
endif

prep_security: prep_nexus
	$(Q_)echo [DIF..... prep_security target ];
	$(Q_)echo [DIF..... building openssl];
	$(Q_)$(MAKE) -j$(NJOBS) -C $(BSEAV_TOP)/opensource/openssl install > $(LOG_REDIRECT_FILE);
	$(Q_)echo [DIF..... building bcrypt];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/security/bcrypt install > $(LOG_REDIRECT_FILE);
	$(Q_)echo [DIF..... building drmrootfs];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/drmrootfs install > $(LOG_REDIRECT_FILE);
	$(Q_)echo [DIF..... building cmndrm];
	$(Q_)$(MAKE) -C $(COMMON_DRM_TOP) install > $(LOG_REDIRECT_FILE)
	$(Q_)echo [DIF..... building cmndrmprdy];
	$(Q_)$(MAKE) -C $(COMMON_DRM_TOP) playready > $(LOG_REDIRECT_FILE)
	$(Q_)echo [DIF..... copying CommonDRM dependent libs...];
	$(Q_)install -c -m 0755 $(COMMON_DRM_BIN_DEPS_LIST) $(NEXUS_BIN_DIR)
	$(Q_)install -c -m 0755 $(B_REFSW_OBJ_ROOT)/BSEAV/lib/security/common_drm/libcmndrmprdy.so $(NEXUS_BIN_DIR)
ifeq ($(SAGE_SUPPORT),y)
	$(Q_)echo [DIF..... building prdyhttp];
	$(Q_)$(MAKE) -C $(BSEAV_TOP)/lib/playready/http install > $(LOG_REDIRECT_FILE)
	$(Q_)echo [DIF..... copying playreadypk 2.5 and 3.0 lib...];
	$(Q_)install -c -m 0755 ${BSEAV_TOP}/$(CMNDRM_PRDY_LIBDIR)/libplayreadypk$(PREBLT_LIB_SUFFIX)_host.so $(NEXUS_BIN_DIR)
	$(Q_)install -c -m 0755 ${BSEAV_TOP}/$(PRDY30_LIBDIR)/libplayready30pk.so $(NEXUS_BIN_DIR)
else
	$(Q_)install -c -m 0755 ${BSEAV_TOP}/$(CMNDRM_PRDY_LIBDIR)/libplayreadypk.so $(NEXUS_BIN_DIR)
endif
