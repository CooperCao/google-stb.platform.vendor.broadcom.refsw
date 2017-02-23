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
ifndef B_PLAYBACK_IP_TARGETS_DEFINED
B_PLAYBACK_IP_TARGETS_DEFINED := y

ifndef NEXUS_TOP
$(error NEXUS_TOP must be defined)
endif

.PHONY: b_playback_ip_target_dummy b_playback_ip_target_build b_playback_ip_target_clean

include $(NEXUS_TOP)/lib/os/b_os_lib_targets.inc
B_PLAYBACK_IP_OTHER_BUILDS += b_os_lib_target_build
B_PLAYBACK_IP_OTHER_CLEANS += b_os_lib_target_clean
B_PLAYBACK_IP_OTHER_CLEAN_ALL += b_os_lib_target_clean_all
B_PLAYBACK_IP_BUILD_FILES += $(B_OS_LIB_BUILD_FILES)
B_OS_LIB_TARGET_MAKEFLAGS += $(B_PLAYBACK_IP_TARGET_MAKEFLAGS)

ifneq ($(LIVEMEDIA_SUPPORT),n)
include $(BSEAV)/opensource/livemedia/b_livemedia_targets.inc
B_PLAYBACK_IP_OTHER_BUILDS += b_livemedia_target_build
B_PLAYBACK_IP_OTHER_CLEANS += b_livemedia_target_clean
B_PLAYBACK_IP_OTHER_CLEAN_ALL += b_livemedia_target_clean_all
B_PLAYBACK_IP_BUILD_FILES += $(B_LIVEMEDIA_BUILD_FILES)
B_LIVEMEDIA_TARGET_MAKEFLAGS += $(B_PLAYBACK_IP_TARGET_MAKEFLAGS)
endif

ifneq ($(SSL_SUPPORT),n)
include $(BSEAV)/opensource/openssl/b_openssl_targets.inc
B_PLAYBACK_IP_OTHER_BUILDS += b_openssl_target_build
B_PLAYBACK_IP_OTHER_CLEANS += b_openssl_target_clean
B_PLAYBACK_IP_OTHER_CLEAN_ALL += b_openssl_target_clean_all
B_PLAYBACK_IP_BUILD_FILES += $(B_OPENSSL_BUILD_FILES)
B_OPENSSL_TARGET_MAKEFLAGS += $(B_PLAYBACK_IP_TARGET_MAKEFLAGS)
endif

b_playback_ip_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

b_playback_ip_target_build: $(B_PLAYBACK_IP_OTHER_BUILDS)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/playback_ip $(sort $(B_PLAYBACK_IP_TARGET_MAKEFLAGS))

b_playback_ip_target_clean: $(B_PLAYBACK_IP_OTHER_CLEANS)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/playback_ip clean $(sort $(B_PLAYBACK_IP_TARGET_MAKEFLAGS))

b_playback_ip_target_clean_all: b_playback_ip_target_clean $(B_PLAYBACK_IP_OTHER_CLEAN_ALL)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
NEXUS_BIN_DIR ?= $(B_REFSW_OBJ_ROOT)/nexus/bin

ifdef B_LIB_STATIC
B_PLAYBACK_IP_BUILD_FILES += $(NEXUS_BIN_DIR)/libb_playback_ip.a
else
B_PLAYBACK_IP_BUILD_FILES += $(NEXUS_BIN_DIR)/libb_playback_ip.so
endif

endif #B_PLAYBACK_IP_TARGETS_DEFINED
