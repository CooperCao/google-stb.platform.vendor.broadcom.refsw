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

ifndef B_bmon_plugin_power_TARGETS_DEFINED
B_bmon_plugin_power_TARGETS_DEFINED := y

$(if $(B_REFSW_TOP),,  $(error ERROR: B_REFSW_TOP is not defined))
BCHP_CHIP_TMP := $(subst 9,,$(NEXUS_PLATFORM))
BCHP_CHIP?=$(BCHP_CHIP_TMP)
#
# If this makefile is called from the parent directory, then PWD and B_THIS_DIR will be pointing
# to the parent directory and not this working directory.
SRC_DIR = $(NEXUS_TOP)/../BSEAV/tools/bmon/plugins/power

# Define any prerequisite sub-packages here:
# For example:
# NxServer:
include $(B_REFSW_TOP)/nexus/nxclient/server/b_nxserver_targets.inc
B_bmon_plugin_power_SUB_BUILDS += b_nxserver_target_build
B_bmon_plugin_power_SUB_CLEANS += b_nxserver_target_clean
B_bmon_plugin_power_SUB_CLEAN_ALL += b_nxserver_target_clean_all
B_bmon_plugin_power_BUILD_FILES += $(B_NXSERVER_BUILD_FILES)

# cJSON:
include $(B_REFSW_TOP)/BSEAV/opensource/cjson/b_libcjson_targets.inc
include $(B_REFSW_TOP)/BSEAV/opensource/cjson/cjson.inc
B_bmon_plugin_power_SUB_BUILDS += b_cjson_target_build
B_bmon_plugin_power_SUB_CLEANS += b_cjson_target_clean
B_bmon_plugin_power_SUB_CLEAN_ALL += b_cjson_target_clean_all
B_bmon_plugin_power_BUILD_FILES += $(B_CJSON_BUILD_FILES)

B_bmon_plugin_power_BUILD_DIR ?= $(B_REFSW_OBJ_ROOT)/BSEAV/tools/bmon/plugins/power
B_bmon_plugin_power_DEST_DIR ?= $(B_REFSW_OBJ_ROOT)/nexus/bin/plugins

B_bmon_plugin_power_NXCLIENT_DIR = $(B_REFSW_OBJ_ROOT)/nexus/bin/nxclient

${B_bmon_plugin_power_NXCLIENT_DIR}:
	@echo "mkdir ... B_bmon_plugin_power_NXCLIENT_DIR ${B_bmon_plugin_power_NXCLIENT_DIR}"
	@mkdir -p ${B_bmon_plugin_power_NXCLIENT_DIR};

${B_bmon_plugin_power_BUILD_DIR}:
	@mkdir -p ${B_bmon_plugin_power_BUILD_DIR};

${B_bmon_plugin_power_DEST_DIR}:
	@echo "mkdir ... B_bmon_plugin_power_DEST_DIR ${B_bmon_plugin_power_DEST_DIR}"
	@mkdir -p ${B_bmon_plugin_power_DEST_DIR};

${B_bmon_plugin_power_BUILD_DIR}/power_tree.h:
ifeq ($(findstring $(BCHP_CHIP),7429 7435 7425 7231 7346 7584 7278 ),$(BCHP_CHIP))
	@echo "CLK_TREE_REG_INFO registerInfo[] = {{\"none\",\"none\",0,0,0,0 }};" > ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h;
else
	@test ! -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || echo "[Exists ... power_regs.h]" > /dev/null;
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || echo "[Creating ... power_regs.h]";
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || echo "[mkdir ... ${B_bmon_plugin_power_DEST_DIR}]";
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || mkdir -p ${B_bmon_plugin_power_DEST_DIR};
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || echo "[awk ... power1.awk]";
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || awk -f ${SRC_DIR}/power1.awk ${SRC_DIR}/power_$(BCHP_CHIP)_$(BCHP_VER)_tree.txt | sort -u > ${B_bmon_plugin_power_BUILD_DIR}/temp.c;
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || echo "[awk ... power2.awk]";
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || awk -f ${SRC_DIR}/power2.awk ${B_bmon_plugin_power_BUILD_DIR}/temp.c  > ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h;
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || echo "[rm ... temp.c]";
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_regs.h || rm -f ${B_bmon_plugin_power_BUILD_DIR}/temp.c;
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_tree.h || cp ${SRC_DIR}/power_$(BCHP_CHIP)_$(BCHP_VER)_tree.txt $(B_REFSW_OBJ_ROOT)/nexus/bin/plugins/power_tree.txt
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_tree.h || cp ${SRC_DIR}/power_$(BCHP_CHIP)_$(BCHP_VER)_clk_tree.txt $(B_REFSW_OBJ_ROOT)/nexus/bin/plugins/power_clk_tree.txt
endif

.PHONY: b_bmon_plugin_power_target_dummy
b_bmon_plugin_power_target_dummy:
	@echo "Error - please define your default rule before including any b_xxx_targets.inc files"
	@false

.PHONY: b_bmon_plugin_power_target_build

b_bmon_plugin_power_target_build: ${B_bmon_plugin_power_BUILD_DIR} ${B_bmon_plugin_power_DEST_DIR} ${B_bmon_plugin_power_NXCLIENT_DIR} ${B_bmon_plugin_power_BUILD_DIR}/power_tree.h $(B_bmon_plugin_power_SUB_BUILDS)
	$(Q_)$(MAKE) -C $(B_REFSW_TOP)/BSEAV/tools/bmon/plugins/power $(sort $(B_bmon_plugin_power_TARGET_MAKEFLAGS)) build_without_subs
	$(Q_)$(CP) -f $(B_REFSW_TOP)/nexus/nxclient/apps/resources/thermal.cfg ${B_bmon_plugin_power_NXCLIENT_DIR}

.PHONY: b_bmon_plugin_power_target_clean
b_bmon_plugin_power_target_clean: $(B_bmon_plugin_power_SUB_CLEANS)
	$(Q_)$(MAKE) -C $(B_REFSW_TOP)/BSEAV/tools/bmon/plugins/power $(sort $(B_bmon_plugin_power_TARGET_MAKEFLAGS)) clean_without_subs
	@test   -f ${B_bmon_plugin_power_BUILD_DIR}/power_tree.h || rm -f ${B_bmon_plugin_power_BUILD_DIR}/power_tree.h

.PHONY: b_bmon_plugin_power_target_clean_all
b_bmon_plugin_power_target_clean_all: b_bmon_plugin_power_target_clean $(B_bmon_plugin_power_SUB_CLEAN_ALL)

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_TOP}/${B_REFSW_OBJ_DIR}
B_bmon_plugin_power_BUILD_FILES += $(B_REFSW_OBJ_ROOT)/tools/bmon/plugins/power/power

endif # B_bmon_plugin_power_TARGETS_DEFINED
