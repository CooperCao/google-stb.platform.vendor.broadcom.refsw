############################################################
#  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
ifndef B_DTCP_IP_SAGE_LIB_TARGETS_DEFINED
B_DTCP_IP_SAGE_LIB_TARGETS_DEFINED := y

ifndef NEXUS_TOP
$(error NEXUS_TOP must be defined)
endif

.PHONY: b_dtcp_ip_sage_lib_target_dummy b_dtcp_ip_sage_lib_target_build b_dtcp_ip_sage_lib_target_clean

BSEAV ?= $(abspath $(NEXUS_TOP)/../BSEAV)

include $(BSEAV)/lib/drmrootfs/b_drmrootfs_targets.inc
B_DTCP_IP_SAGE_OTHER_BUILDS += b_drmrootfs_target_build
B_DTCP_IP_SAGE_OTHER_CLEANS += b_drmrootfs_target_clean
B_DTCP_IP_SAGE_OTHER_CLEAN_ALL += b_drmrootfs_target_clean_all
B_DTCP_IP_SAGE_BUILD_FILES += $(B_DRMROOTFS_BUILD_FILES)
B_DRMROOTFS_TARGET_MAKEFLAGS += $(B_DTCP_IP_SAGE_TARGET_MAKEFLAGS)

include $(NEXUS_TOP)/../BSEAV/lib/openssl/b_openssl_targets.inc
B_DTCP_IP_SAGE_OTHER_BUILDS += b_openssl_target_build
B_DTCP_IP_SAGE_OTHER_CLEANS += b_openssl_target_clean
B_DTCP_IP_SAGE_OTHER_CLEAN_ALL += b_openssl_target_clean_all
B_DTCP_IP_SAGE_BUILD_FILES += $(B_OPENSSL_BUILD_FILES)
B_OPENSSL_TARGET_MAKEFLAGS += $(B_DTCP_IP_SAGE_TARGET_MAKEFLAGS)

# If DTCP_IP_SUPPORT is requested
ifeq ($(DTCP_IP_SAGE_SUPPORT),y)
 ifeq ($(SSL_SUPPORT),n)
  $(error Cannot set DTCP_IP_SAGE_SUPPORT=y and SSL_SUPPORT=n)
 endif
 # If we have DTCP sources
 ifeq ($(shell test -e $(NEXUS_TOP)/lib/dtcp_ip_sage/src/b_dtcp_applib.c   && echo y),y)
  include $(BSEAV)/lib/security/bcrypt/b_bcrypt_targets.inc
  include $(BSEAV)/lib/security/common_drm/b_common_drm_targets.inc
  B_DTCP_IP_SAGE_OTHER_BUILDS += b_bcrypt_target_build b_common_drm_target_build
  B_DTCP_IP_SAGE_OTHER_CLEANS += b_bcrypt_target_clean b_common_drm_target_clean
  B_DTCP_IP_SAGE_OTHER_CLEAN_ALL += b_bcrypt_target_clean_all b_common_drm_target_clean_all
  B_DTCP_IP_SAGE_BUILD_FILES += $(B_BCRYPT_BUILD_FILES) $(B_COMMON_DRM_BUILD_FILES)
  B_BCRYPT_TARGET_MAKEFLAGS += $(B_DTCP_IP_SAGE_TARGET_MAKEFLAGS)
  B_COMMON_DRM_TARGET_MAKEFLAGS += $(B_DTCP_IP_SAGE_TARGET_MAKEFLAGS)
 endif
endif

b_dtcp_ip_sage_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

b_dtcp_ip_sage_target_build: $(B_DTCP_IP_SAGE_OTHER_BUILDS)
	@echo [Build..... dtcp_ip_sage]
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/dtcp_ip_sage B_BUILD_DEPS=n $(sort $(B_DTCP_IP_SAGELIB_TARGET_MAKEFLAGS))

b_dtcp_ip_sage_target_clean: $(B_DTCP_IP_SAGE_OTHER_CLEANS)
	@echo [Clean..... dtcp_ip_sage]
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/dtcp_ip_sage clean B_BUILD_DEPS=n $(sort $(B_DTCP_IP_SAGELIB_TARGET_MAKEFLAGS))

b_dtcp_ip_sage_target_clean_all: b_dtcp_ip_sage_target_clean $(B_DTCP_IP_SAGE_OTHER_CLEAN_ALL)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
NEXUS_BIN_DIR ?= $(B_REFSW_OBJ_ROOT)/nexus/bin

ifeq ($(DTCP_IP_SUPPORT),y)
 ifdef B_LIB_STATIC
  B_DTCP_IP_BUILD_FILES += $(NEXUS_BIN_DIR)/libb_dtcp_ip.a
 else
  B_DTCP_IP_BUILD_FILES += $(NEXUS_BIN_DIR)/libb_dtcp_ip.so
 endif
else
 B_DTCP_IP_BUILD_FILES += $(NEXUS_BIN_DIR)/libb_dtcp_ip_stubs.a
endif

endif #B_DTCP_IP_SAGELIB_TARGETS_DEFINED
