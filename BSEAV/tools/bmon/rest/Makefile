###############################################################################
#   Copyright (C) 2018 Broadcom.
#   The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#   This program is the proprietary software of Broadcom and/or its licensors,
#   and may only be used, duplicated, modified or distributed pursuant to
#   the terms and conditions of a separate, written license agreement executed
#   between you and Broadcom (an "Authorized License").  Except as set forth in
#   an Authorized License, Broadcom grants no license (express or implied),
#   right to use, or waiver of any kind with respect to the Software, and
#   Broadcom expressly reserves all rights in and to the Software and all
#   intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#   THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#   IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#   Except as expressly set forth in the Authorized License,
#
#   1.     This program, including its structure, sequence and organization,
#   constitutes the valuable trade secrets of Broadcom, and you shall use all
#   reasonable efforts to protect the confidentiality thereof, and to use this
#   information only in connection with your use of Broadcom integrated circuit
#   products.
#
#   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#   "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#   OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#   RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#   IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#   A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#   ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#   THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#   OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#   INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#   RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#   EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#   WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#   FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################

B_THIS_DIR:=BSEAV/tools/bmon/rest

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif
export NEXUS_BMON_SUPPORT=y

ifeq ($(B_REFSW_REAL_MAKE),)
NEXUS_TOP ?= $(subst /${B_THIS_DIR},,$(CURDIR))/nexus

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
NEXUS_BIN_DIR := ${B_REFSW_OBJ_ROOT}/nexus/bin

else # B_REFSW_REAL_MAKE
NEXUS_TOP?=${B_REFSW_REAL_NEXUS_TOP}

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}
NEXUS_BIN_DIR := ${B_REFSW_OBJ_ROOT}/nexus/bin

endif

BIP_SERVER_APP := true

NXCLIENT_SUPPORT_LOWERCASE = $(shell awk 'BEGIN{print tolower("$(NXCLIENT_SUPPORT)")}')

NEXUS_BIN_DIR ?= -L$(abspath $(B_REFSW_OBJ_ROOT)/nexus/bin)
ifeq ($(NXCLIENT_SUPPORT_LOWERCASE),y)
THEAPPS += rest
rest: fileread libplugin_launcher
else
rest:
	@echo [NOTE: NXCLIENT_SUPPORT is NOT set. Is is required for BIP which is required for rest.]
endif


fileread:
	@echo [Building ... $(notdir $@)]
	@make -C ${NEXUS_TOP}/../BSEAV/tools/bmon/plugins/fileread -f ${NEXUS_TOP}/../BSEAV/tools/bmon/plugins/fileread/Makefile
	@cp -pf ${NEXUS_BIN_DIR}/plugins/$@ ${NEXUS_BIN_DIR}

libplugin_launcher:
	@echo [Building ... $(notdir $@)]
	@make -C ${NEXUS_TOP}/../BSEAV/tools/bmon/plugin_launcher -f ${NEXUS_TOP}/../BSEAV/tools/bmon/plugin_launcher/Makefile

CFLAGS += -I$(NEXUS_TOP)/lib/bip/src
CFLAGS += -I$(NEXUS_TOP)/lib/playback_ip/include
###CFLAGS += -DAPPEND_PERFORMANCE_DATA
CFLAGS += -I$(NEXUS_TOP)/../$(B_THIS_DIR)/../common
CFLAGS += -I$(NEXUS_TOP)/../$(B_THIS_DIR)/../plugin_launcher
LDFLAGS += $(PLUGIN_LAUNCHER_LDFLAGS)

include ${NEXUS_TOP}/lib/bip/examples/Makefile.inc
include ${NEXUS_TOP}/../BSEAV/tools/bmon/plugin_launcher/plugin_launcher.inc
include ${NEXUS_TOP}/../BSEAV/tools/bmon/plugin_launcher/b_plugin_launcher_targets.inc
