############################################################
#  Copyright (C) 2018 Broadcom.
#  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to
#  the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied),
#  right to use, or waiver of any kind with respect to the Software, and
#  Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
############################################################
ifndef B_BMON_SCHEDULER_TARGETS_DEFINED
B_BMON_SCHEDULER_TARGETS_DEFINED := y

ifndef NEXUS_TOP
$(error NEXUS_TOP must be defined)
endif

.PHONY: b_bmon_scheduler_target_dummy b_bmon_scheduler_target_build b_bmon_scheduler_target_clean

include $(NEXUS_TOP)/lib/os/b_os_lib_targets.inc
B_BMON_SCHEDULER_OTHER_BUILDS += b_os_lib_target_build
B_BMON_SCHEDULER_OTHER_CLEANS += b_os_lib_target_clean
B_BMON_SCHEDULER_OTHER_CLEAN_ALL += b_os_lib_target_clean_all
B_BMON_SCHEDULER_BUILD_FILES += $(B_OS_LIB_BUILD_FILES)
B_OS_LIB_TARGET_MAKEFLAGS += $(B_BMON_SCHEDULER_TARGET_MAKEFLAGS)

include $(NEXUS_TOP)/../BSEAV/opensource/cjson/b_libcjson_targets.inc
B_BMON_SCHEDULER_OTHER_BUILDS += b_cjson_target_build
B_BMON_SCHEDULER_OTHER_CLEANS += b_cjson_target_clean
B_BMON_SCHEDULER_OTHER_CLEAN_ALL += b_cjson_target_clean_all
B_BMON_SCHEDULER_BUILD_FILES += $(B_CJSON_BUILD_FILES)
B_CJSON_TARGET_MAKEFLAGS += $(B_BMON_SCHEDULER_TARGET_MAKEFLAGS)

include $(NEXUS_TOP)/../BSEAV/tools/bmon/plugin_launcher/b_plugin_launcher_targets.inc
B_BMON_SCHEDULER_OTHER_BUILDS += b_plugin_launcher_target_build
B_BMON_SCHEDULER_OTHER_CLEANS += b_plugin_launcher_target_clean
B_BMON_SCHEDULER_OTHER_CLEAN_ALL += b_plugin_launcher_target_clean_all
B_BMON_SCHEDULER_BUILD_FILES += $(B_PLUGIN_LAUNCHER_BUILD_FILES)
B_PLUGIN_LAUNCHER_TARGET_MAKEFLAGS += $(B_BMON_SCHEDULER_TARGET_MAKEFLAGS)

b_bmon_scheduler_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

b_bmon_scheduler_target_build: $(B_BMON_SCHEDULER_OTHER_BUILDS)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/tools/bmon/scheduler -f Makefile.lib $(sort $(B_BMON_SCHEDULER_TARGET_MAKEFLAGS))

b_bmon_scheduler_target_clean: $(B_BMON_SCHEDULER_OTHER_CLEANS)
	$(Q_)$(MAKE) -C $(NEXUS_TOP)/../BSEAV/tools/bmon/scheduler -f Makefile.lib clean $(sort $(B_BMON_SCHEDULER_TARGET_MAKEFLAGS))

b_bmon_scheduler_target_clean_all: b_bmon_scheduler_target_clean $(B_BMON_SCHEDULER_OTHER_CLEAN_ALL)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
NEXUS_BIN_DIR ?= $(B_REFSW_OBJ_ROOT)/nexus/bin

ifdef B_LIB_STATIC
B_BMON_SCHEDULER_BUILD_FILES += $(NEXUS_BIN_DIR)/libbmon_scheduler.a
else
B_BMON_SCHEDULER_BUILD_FILES += $(NEXUS_BIN_DIR)/libbmon_scheduler.so
endif

endif #B_BMON_SCHEDULER_TARGETS_DEFINED
