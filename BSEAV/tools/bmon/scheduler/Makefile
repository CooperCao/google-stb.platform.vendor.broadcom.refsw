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
#
# Module Description:
#   This file defines the rules to build the scheduler library
#
############################################################

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../../nexus/lib && cd)
else
B_LIB_TOP := $(shell cd ../../../../nexus/lib; pwd)
endif
BSEAV = $(B_LIB_TOP)/../../BSEAV
NEXUS = $(B_LIB_TOP)/../../nexus
NEXUS_TOP = $(NEXUS)
B_THIS_DIR := BSEAV/tools/bmon/scheduler
SCHEDULER_BIN_DIR = $(NEXUS_BIN_DIR)

# Identify object directory
B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
B_LIB_OBJ_PREFIX ?= nexus/lib/
B_LIB_OBJ_DIR ?= ${B_REFSW_OBJ_ROOT}/$(B_LIB_OBJ_PREFIX)$(B_REFSW_ARCH)/$(B_LIB_OUTPUT_NAME)

.PHONY = scheduler_test

NEXUS_PLATFORM ?= $(PLATFORM)
NXCLIENT_SUPPORT ?= y
ifeq ($(NEXUS_PLATFORM),)
$(error Please define NEXUS_PLATFORM first)
endif
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# default rule to build libbmon_scheduler.a/.so libs.
scheduler_test: $(NEXUS_BIN_DIR)/libnexus.so b_bmon_scheduler_target_build main.c
	@echo [Compiling ... $@]
	$(Q_)$(CC) -o $(SCHEDULER_BIN_DIR)/$@ main.c $(APP_CFLAGS) $(APP_LDFLAGS) -std=c99

$(NEXUS_BIN_DIR)/libnexus.so:
	$(MAKE) -C $(NEXUS)/build

# Include module sources
# Note: You must also include any dependent libraries here also.
include $(B_LIB_TOP)/os/b_os_lib.inc
include $(B_LIB_TOP)/os/b_os_lib_targets.inc
include $(BSEAV)/opensource/cjson/cjson.inc
include $(BSEAV)/opensource/cjson/b_libcjson_targets.inc
PLUGIN_LAUNCHER_TOP = $(BSEAV)/tools/bmon/plugin_launcher
include $(BSEAV)/tools/bmon/plugin_launcher/plugin_launcher.inc
include $(BSEAV)/tools/bmon/plugin_launcher/b_plugin_launcher_targets.inc
include b_bmon_scheduler.inc
include b_bmon_scheduler_targets.inc

APP_CFLAGS += \
		-g -D_LINUX_TYPES_H \
		-I$(BSEAV)/tools/bmon/common \
		$(CJSON_CFLAGS) \
		$(NEXUS_CFLAGS) \
		$(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) \
		$(addprefix -D,$(NEXUS_APP_DEFINES)) \
		$(addprefix -I,$(B_OS_LIB_PUBLIC_INCLUDES)) \
		$(PLUGIN_LAUNCHER_CFLAGS) \
                $(B_BMON_SCHEDULER_CFLAGS)

APP_LDFLAGS += \
		-pthread -lm -L$(NEXUS_BIN_DIR) \
		$(NEXUS_BIN_DIR)/libnexus.so \
		$(CJSON_LDFLAGS) \
		$(B_OS_LIB_BUILD_FILES) \
		$(PLUGIN_LAUNCHER_LDFLAGS) \
		$(B_BMON_SCHEDULER_LDFLAGS)
