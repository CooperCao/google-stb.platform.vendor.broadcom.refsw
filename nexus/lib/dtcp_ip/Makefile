#############################################################################
#    (c)2012-2015 Broadcom Corporation
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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################
#Module descriptioin: DTCP-IP Makefile for nexus build
#
#Revision History:
#Created:
#
##############################################################

B_LIB_TOP := $(shell cd ../; pwd)
NEXUS_TOP := $(B_LIB_TOP)/..

NEXUS_PLATFORM ?= $(PLATFORM)
B_REFSW_OS ?= $(OS)

DTCP_IP_INSTALL_DIR ?= ${NEXUS_BIN_DIR}

ifeq ($(NEXUS_PLATFORM),)
$(error NEXUS_PLATFORM has not been defined)
endif

# Include Nexus Defs
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

include dtcp_ip_lib.inc

.PHONY: all
all:  install

.PHONY: prep_bcrypt
prep_bcrypt:
    ifeq (${B_DTCP_IP_BUILD_MODE},stubs)
		@echo [Skipping... bcrypt]
    else
		@echo [Building... bcrypt]
		$(Q_)$(MAKE) -C $(B_LIB_TOP)/../../BSEAV/lib/security/bcrypt install
		$(Q_)$(MAKE) -C $(B_LIB_TOP)/../../BSEAV/lib/security/bcrypt copy
    endif

.PHONY: install
install: prep_bcrypt
	@echo [Building... dtcp_ip_lib \(from $(B_DTCP_IP_BUILD_MODE)\)]
	$(Q_)$(MAKE) -f Makefile.dtcp_ip_lib  B_DTCP_IP_LIB_SOURCES="$(B_DTCP_IP_LIB_SOURCES)" install
    ifeq (${B_DTCP_IP_BUILD_MODE},stubs)
		@echo [Skipping... Common DRM binaries]
    else
	    @echo [..... Common DRM target with j option]
	    $(Q_)$(MAKE) -C $(B_LIB_TOP)/../../BSEAV/lib/security/common_drm install

		@echo [Install... Common DRM binaries]
		$(Q_)$(MKDIR) $(DTCP_IP_INSTALL_DIR)
		$(Q_)cp -dfpu  $(COMMON_DRM_BIN_DEPS_LIST) $(DTCP_IP_INSTALL_DIR)
    endif

.PHONY: clean
clean:
	@echo [Cleaning... dtcp_ip_lib]
	$(Q_)$(MAKE) -f Makefile.dtcp_ip_lib  B_DTCP_IP_LIB_SOURCES="$(B_DTCP_IP_LIB_SOURCES)" clean
    ifeq (${B_DTCP_IP_BUILD_MODE},stubs)
		@echo [Skipping... bcrypt]
    else
		@echo [Cleaning... bcrypt]
		$(Q_)$(MAKE) -C $(B_LIB_TOP)/../../BSEAV/lib/security/bcrypt clean
	    $(Q_)$(MAKE) -C $(B_LIB_TOP)/../../BSEAV/lib/security/common_drm clean
    endif
