############################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
############################################################
ifndef B_ASP_LIB_TARGETS_DEFINED
B_ASP_LIB_TARGETS_DEFINED := y

ifndef NEXUS_TOP
$(error NEXUS_TOP must be defined)
endif

#define build, clean, & files related targets for this library.
#build target includes recipe to build only this library
#User of this lib will add these targets as prerequisites.

#Also, include targets.inc file for all ibraries this lib is dependent on.
#no submake of dependent libs are invoked

.PHONY: b_asp_lib_target_dummy b_asp_lib_target_build b_asp_lib_target_clean

#include our inc file
include $(NEXUS_TOP)/lib/asp/b_asp_lib.inc

include $(NEXUS_TOP)/lib/os/b_os_lib_targets.inc
B_ASP_LIB_OTHER_BUILDS += b_os_lib_target_build
B_ASP_LIB_OTHER_CLEANS += b_os_lib_target_clean
B_ASP_LIB_OTHER_CLEAN_ALL += b_os_lib_target_clean_all
B_ASP_LIB_BUILD_FILES += $(B_OS_LIB_BUILD_FILES)

b_asp_lib_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

    #TODO: add B_BUILD_DEPS=n to make below?
b_asp_lib_target_build: $(B_ASP_LIB_OTHER_BUILDS)
	@echo [Build..... b_asp_lib]
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/asp $(sort $(B_ASP_LIB_TARGET_MAKEFLAGS))

b_asp_lib_target_clean: $(B_ASP_LIB_OTHER_CLEANS)
	@echo [Clean..... b_asp_lib]
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/lib/asp clean $(sort $(B_ASP_LIB_TARGET_MAKEFLAGS))

b_asp_lib_target_clean_all: b_asp_lib_target_clean $(B_ASP_LIB_OTHER_CLEAN_ALL)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
NEXUS_BIN_DIR ?= $(B_REFSW_OBJ_ROOT)/nexus/bin

ifdef B_LIB_STATIC
B_ASP_LIB_BUILD_FILES += $(NEXUS_BIN_DIR)/libb_asp.a
else
B_ASP_LIB_BUILD_FILES += $(NEXUS_BIN_DIR)/libb_asp.so
endif

endif #B_ASPLIB_TARGETS_DEFINED
